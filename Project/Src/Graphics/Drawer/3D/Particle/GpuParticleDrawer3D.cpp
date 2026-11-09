#include "GpuParticleDrawer3D.h"

#include "Graphics/Camera/System/Camera3DSystem.h"
#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/DirectX/ResourceFactory/ResourceFactory.h"
#include "Graphics/DirectX/ViewManagers/SRVUAVManager/SRVUAVManager.h"
#include "Graphics/Pipeline/Manager/PipelineManager.h"
#include "Graphics/Pipeline/Pipeline.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>

namespace {
	constexpr uint32_t kComputeThreadCount = 64u;
	constexpr uint32_t kMaximumMeshDispatchCount = 65535u;
	constexpr uint32_t kConstantBufferSize = 256u;

	uint32_t DispatchCount(uint32_t elementCount) {
		return (elementCount + kComputeThreadCount - 1u) / kComputeThreadCount;
	}

	void AdvanceGeneration(uint32_t& generation) {
		if (++generation == 0u) {
			++generation;
		}
	}

	D3D12_RESOURCE_BARRIER MakeUavBarrier(ID3D12Resource* resource) {
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		barrier.UAV.pResource = resource;
		return barrier;
	}

	D3D12_RESOURCE_BARRIER MakeTransitionBarrier(
		ID3D12Resource* resource,
		D3D12_RESOURCE_STATES before,
		D3D12_RESOURCE_STATES after) {
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = resource;
		barrier.Transition.StateBefore = before;
		barrier.Transition.StateAfter = after;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		return barrier;
	}
}

namespace LGF {

	GpuParticleDrawer3D::GpuParticleDrawer3D(Graphics3DContext context) :
		context_(context) {
	}

	GpuParticleDrawer3D::~GpuParticleDrawer3D() = default;

	bool GpuParticleDrawer3D::Initialize() {
		initializePipeline_ = context_.pipelineMgr.GetPipeline("GpuParticleInitialize3D");
		resetStatisticsPipeline_ = context_.pipelineMgr.GetPipeline("GpuParticleResetStatistics3D");
		spawnPipeline_ = context_.pipelineMgr.GetPipeline("GpuParticleSpawn3D");
		updatePipeline_ = context_.pipelineMgr.GetPipeline("GpuParticleUpdate3D");
		drawPipeline_ = context_.pipelineMgr.GetPipeline("GpuParticleBillboard3D");
		return initializePipeline_ && initializePipeline_->IsValid() &&
			resetStatisticsPipeline_ && resetStatisticsPipeline_->IsValid() &&
			spawnPipeline_ && spawnPipeline_->IsValid() &&
			updatePipeline_ && updatePipeline_->IsValid() &&
			drawPipeline_ && drawPipeline_->IsValid();
	}

	void GpuParticleDrawer3D::BeginFrame() {
		const uint32_t frameIndex = context_.directX.GetCurrentFrameContextIndex();
		for (WorldSlot& slot : worlds_) {
			if (!slot.world) {
				continue;
			}
			World& world = *slot.world;
			// BeginFrame時点で同じFrameContextのFenceは完了済み。
			// ここで読む値は、2フレーム前に記録した非同期スナップショットになる。
			ReadStatistics(world, frameIndex);
			world.spawnBatches.clear();
			world.hasUpdate = false;
			world.shouldDraw = false;
		}
	}

	void GpuParticleDrawer3D::EndFrame() {
		const uint32_t frameIndex = context_.directX.GetCurrentFrameContextIndex();
		for (WorldSlot& slot : worlds_) {
			if (!slot.world) {
				continue;
			}
			World& world = *slot.world;
			if (world.needsInitialization) {
				DispatchInitialize(world);
				world.needsInitialization = false;
			}
			if (world.hasUpdate) {
				WriteFrameData(world, frameIndex);
				DispatchSimulation(world, frameIndex);
			}
			if (world.shouldDraw) {
				DispatchDraw(world);
			}
			// Updateされなかったフレームでも、最後の生存数を非同期で回収する。
			CopyStatistics(world, frameIndex);
		}
	}

	void GpuParticleDrawer3D::Finalize() {
		worlds_.clear();
		freeWorldIndices_.clear();
	}

	GpuParticleWorldHandle GpuParticleDrawer3D::CreateWorld(const GpuParticleWorldDesc& desc) {
		if (desc.maximumParticleCount == 0u ||
			desc.maximumParticleCount > kMaximumMeshDispatchCount ||
			desc.maximumSpawnBatchCount == 0u ||
			desc.maximumSpawnBatchCount > kMaximumMeshDispatchCount ||
			!std::isfinite(desc.boundsPadding) || desc.boundsPadding < 0.0f) {
			return {};
		}

		uint32_t index{};
		if (freeWorldIndices_.empty()) {
			index = static_cast<uint32_t>(worlds_.size());
			worlds_.emplace_back();
		}
		else {
			index = freeWorldIndices_.back();
			freeWorldIndices_.pop_back();
		}

		WorldSlot& slot = worlds_[index];
		slot.world = std::make_unique<World>();
		slot.world->desc = desc;
		if (!InitializeWorld(*slot.world)) {
			slot.world.reset();
			freeWorldIndices_.push_back(index);
			return {};
		}
		return { index, slot.generation };
	}

	bool GpuParticleDrawer3D::RemoveWorld(GpuParticleWorldHandle handle) {
		World* world = FindWorld(handle);
		if (!world) {
			return false;
		}
		// World破棄は頻繁に行わない操作なので、試作版では安全性を優先してGPUを待つ。
		context_.directX.WaitIdle();
		WorldSlot& slot = worlds_[handle.index];
		auto* views = context_.directX.GetResMgr().srvuav;
		views->Release(world->particleUavIndex);
		views->Release(world->freeListUavIndex);
		views->Release(world->counterUavIndex);
		for (const uint32_t spawnSrvIndex : world->spawnSrvIndices) {
			views->Release(spawnSrvIndex);
		}
		slot.world.reset();
		AdvanceGeneration(slot.generation);
		freeWorldIndices_.push_back(handle.index);
		return true;
	}

	bool GpuParticleDrawer3D::ClearWorld(GpuParticleWorldHandle handle) {
		World* world = FindWorld(handle);
		if (!world) {
			return false;
		}
		world->spawnBatches.clear();
		world->latestStatistics = {};
		world->needsInitialization = true;
		return true;
	}

	bool GpuParticleDrawer3D::UpdateWorld(
		GpuParticleWorldHandle handle,
		float deltaTime,
		std::span<const GpuParticleSpawnBatch3D> spawnBatches) {
		World* world = FindWorld(handle);
		if (!world || !std::isfinite(deltaTime) || deltaTime < 0.0f ||
			spawnBatches.size() > world->desc.maximumSpawnBatchCount) {
			return false;
		}
		world->deltaTime = deltaTime;
		world->spawnBatches.assign(spawnBatches.begin(), spawnBatches.end());
		world->hasUpdate = true;
		return true;
	}

	bool GpuParticleDrawer3D::DrawWorld(GpuParticleWorldHandle handle) {
		World* world = FindWorld(handle);
		if (!world) {
			return false;
		}
		world->shouldDraw = true;
		return true;
	}

	GpuParticleStatistics3D GpuParticleDrawer3D::GetStatistics(GpuParticleWorldHandle handle) const {
		const World* world = FindWorld(handle);
		return world ? world->latestStatistics : GpuParticleStatistics3D{};
	}

	GpuParticleDrawer3D::World* GpuParticleDrawer3D::FindWorld(GpuParticleWorldHandle handle) {
		return const_cast<World*>(std::as_const(*this).FindWorld(handle));
	}

	const GpuParticleDrawer3D::World* GpuParticleDrawer3D::FindWorld(GpuParticleWorldHandle handle) const {
		if (!handle.IsValid() || handle.index >= worlds_.size()) {
			return nullptr;
		}
		const WorldSlot& slot = worlds_[handle.index];
		return slot.world && slot.generation == handle.generation ? slot.world.get() : nullptr;
	}

	bool GpuParticleDrawer3D::InitializeWorld(World& world) {
		auto* factory = context_.directX.GetResourceFactory();
		auto* views = context_.directX.GetResMgr().srvuav;
		const uint32_t particleCount = world.desc.maximumParticleCount;
		const uint32_t requiredViewCount = 3u + kFrameContextCount;
		if (!views->CanAllocate(requiredViewCount)) {
			return false;
		}

		world.particleResource = factory->CreateUAVBufferResource(sizeof(ParticleForGPU) * particleCount);
		world.freeListResource = factory->CreateUAVBufferResource(sizeof(uint32_t) * particleCount);
		world.counterResource = factory->CreateUAVBufferResource(sizeof(CountersForGPU));
		if (!world.particleResource || !world.freeListResource || !world.counterResource) {
			return false;
		}

		world.particleUavIndex = views->Allocate();
		world.freeListUavIndex = views->Allocate();
		world.counterUavIndex = views->Allocate();
		views->CreateUavStructuredBuffer(
			world.particleUavIndex, world.particleResource.Get(), particleCount, sizeof(ParticleForGPU));
		views->CreateUavStructuredBuffer(
			world.freeListUavIndex, world.freeListResource.Get(), particleCount, sizeof(uint32_t));
		views->CreateUavStructuredBuffer(
			world.counterUavIndex, world.counterResource.Get(), 1u, sizeof(CountersForGPU));

		const bool spawnInitialized = world.spawnResources.Initialize(
			*factory, sizeof(SpawnBatchForGPU) * world.desc.maximumSpawnBatchCount);
		const bool constantInitialized = world.constantResources.Initialize(*factory, kConstantBufferSize);
		if (!spawnInitialized || !constantInitialized) {
			return false;
		}
		for (uint32_t frameIndex = 0u; frameIndex < kFrameContextCount; ++frameIndex) {
			world.spawnSrvIndices[frameIndex] = views->Allocate();
			views->CreateSrvStructuredBuffer(
				world.spawnSrvIndices[frameIndex],
				world.spawnResources.GetResource(frameIndex),
				world.desc.maximumSpawnBatchCount,
				sizeof(SpawnBatchForGPU));
			world.readbackResources[frameIndex] = factory->CreateReadbackBufferResource(sizeof(CountersForGPU));
			if (!world.readbackResources[frameIndex]) {
				return false;
			}
		}
		return true;
	}

	void GpuParticleDrawer3D::ReadStatistics(World& world, uint32_t frameIndex) {
		if (!world.hasReadback[frameIndex]) {
			return;
		}
		CountersForGPU counters{};
		void* mappedData = nullptr;
		const D3D12_RANGE readRange{ 0u, sizeof(CountersForGPU) };
		if (SUCCEEDED(world.readbackResources[frameIndex]->Map(0u, &readRange, &mappedData)) && mappedData) {
			std::memcpy(&counters, mappedData, sizeof(counters));
			const D3D12_RANGE writtenRange{};
			world.readbackResources[frameIndex]->Unmap(0u, &writtenRange);
			world.latestStatistics = {
				.particleCount = counters.aliveCount,
				.generatedCount = counters.generatedCount,
				.rejectedCount = counters.rejectedCount,
				.expiredCount = counters.expiredCount,
				.outOfBoundsCount = counters.outOfBoundsCount,
				.latencyFrames = context_.directX.GetFrameContextCountInUse(),
				.isAvailable = true,
			};
		}
	}

	void GpuParticleDrawer3D::WriteFrameData(World& world, uint32_t frameIndex) {
		auto* spawnData = reinterpret_cast<SpawnBatchForGPU*>(world.spawnResources.GetMappedData(frameIndex));
		for (std::size_t i = 0u; i < world.spawnBatches.size(); ++i) {
			const GpuParticleSpawnBatch3D& source = world.spawnBatches[i];
			SpawnBatchForGPU& destination = spawnData[i];
			destination.position = source.position;
			destination.minimumLifetime = source.minimumLifetime;
			destination.minimumVelocity = source.minimumVelocity;
			destination.maximumLifetime = source.maximumLifetime;
			destination.maximumVelocity = source.maximumVelocity;
			destination.minimumRotation = source.minimumRotation;
			destination.acceleration = source.acceleration;
			destination.maximumRotation = source.maximumRotation;
			destination.startColor = source.startColor;
			destination.endColor = source.endColor;
			destination.startSize = source.startSize;
			destination.endSize = source.endSize;
			destination.minimumAngularVelocity = source.minimumAngularVelocity;
			destination.maximumAngularVelocity = source.maximumAngularVelocity;
			destination.count = source.count;
			destination.randomSeed = source.randomSeed;
			destination.textureIndex = source.texture.srvIndex_;
		}

		auto* constants = reinterpret_cast<ConstantsForGPU*>(world.constantResources.GetMappedData(frameIndex));
		*constants = {
			.worldMinimum = world.desc.worldMinimum,
			.deltaTime = world.deltaTime,
			.worldMaximum = world.desc.worldMaximum,
			.boundsPadding = world.desc.boundsPadding,
			.maximumParticleCount = world.desc.maximumParticleCount,
			.spawnBatchCount = static_cast<uint32_t>(world.spawnBatches.size()),
			.removeOutOfBounds = world.desc.removeOutOfBounds ? 1u : 0u,
		};
	}

	void GpuParticleDrawer3D::DispatchInitialize(World& world) {
		auto* commandList = context_.directX.GetCommandList();
		commandList->SetComputeRootSignature(initializePipeline_->GetRootSignature());
		commandList->SetPipelineState(initializePipeline_->GetPipelineState());
		auto* views = context_.directX.GetResMgr().srvuav;
		commandList->SetComputeRootDescriptorTable(
			initializePipeline_->GetRootIndex("gParticles"), views->GetGPUDescriptorHandle(world.particleUavIndex));
		commandList->SetComputeRootDescriptorTable(
			initializePipeline_->GetRootIndex("gFreeList"), views->GetGPUDescriptorHandle(world.freeListUavIndex));
		commandList->SetComputeRootDescriptorTable(
			initializePipeline_->GetRootIndex("gCounters"), views->GetGPUDescriptorHandle(world.counterUavIndex));
		const uint32_t frameIndex = context_.directX.GetCurrentFrameContextIndex();
		auto* constants = reinterpret_cast<ConstantsForGPU*>(world.constantResources.GetMappedData(frameIndex));
		*constants = { .maximumParticleCount = world.desc.maximumParticleCount };
		commandList->SetComputeRootConstantBufferView(
			initializePipeline_->GetRootIndex("GpuParticleConstants"),
			world.constantResources.GetGPUVirtualAddress(frameIndex));
		commandList->Dispatch(DispatchCount(world.desc.maximumParticleCount), 1u, 1u);
		const D3D12_RESOURCE_BARRIER barriers[] = {
			MakeUavBarrier(world.particleResource.Get()),
			MakeUavBarrier(world.freeListResource.Get()),
			MakeUavBarrier(world.counterResource.Get()),
		};
		commandList->ResourceBarrier(_countof(barriers), barriers);
	}

	void GpuParticleDrawer3D::DispatchSimulation(World& world, uint32_t frameIndex) {
		auto* commandList = context_.directX.GetCommandList();
		auto* views = context_.directX.GetResMgr().srvuav;

		commandList->SetComputeRootSignature(resetStatisticsPipeline_->GetRootSignature());
		commandList->SetPipelineState(resetStatisticsPipeline_->GetPipelineState());
		commandList->SetComputeRootDescriptorTable(
			resetStatisticsPipeline_->GetRootIndex("gCounters"), views->GetGPUDescriptorHandle(world.counterUavIndex));
		commandList->Dispatch(1u, 1u, 1u);
		D3D12_RESOURCE_BARRIER barrier = MakeUavBarrier(world.counterResource.Get());
		commandList->ResourceBarrier(1u, &barrier);

		if (!world.spawnBatches.empty()) {
			commandList->SetComputeRootSignature(spawnPipeline_->GetRootSignature());
			commandList->SetPipelineState(spawnPipeline_->GetPipelineState());
			commandList->SetComputeRootDescriptorTable(
				spawnPipeline_->GetRootIndex("gParticles"), views->GetGPUDescriptorHandle(world.particleUavIndex));
			commandList->SetComputeRootDescriptorTable(
				spawnPipeline_->GetRootIndex("gFreeList"), views->GetGPUDescriptorHandle(world.freeListUavIndex));
			commandList->SetComputeRootDescriptorTable(
				spawnPipeline_->GetRootIndex("gCounters"), views->GetGPUDescriptorHandle(world.counterUavIndex));
			commandList->SetComputeRootDescriptorTable(
				spawnPipeline_->GetRootIndex("gSpawnBatches"), views->GetGPUDescriptorHandle(world.spawnSrvIndices[frameIndex]));
			commandList->SetComputeRootConstantBufferView(
				spawnPipeline_->GetRootIndex("GpuParticleConstants"), world.constantResources.GetGPUVirtualAddress(frameIndex));
			commandList->Dispatch(static_cast<uint32_t>(world.spawnBatches.size()), 1u, 1u);
			const D3D12_RESOURCE_BARRIER spawnBarriers[] = {
				MakeUavBarrier(world.particleResource.Get()),
				MakeUavBarrier(world.freeListResource.Get()),
				MakeUavBarrier(world.counterResource.Get()),
			};
			commandList->ResourceBarrier(_countof(spawnBarriers), spawnBarriers);
		}

		commandList->SetComputeRootSignature(updatePipeline_->GetRootSignature());
		commandList->SetPipelineState(updatePipeline_->GetPipelineState());
		commandList->SetComputeRootDescriptorTable(
			updatePipeline_->GetRootIndex("gParticles"), views->GetGPUDescriptorHandle(world.particleUavIndex));
		commandList->SetComputeRootDescriptorTable(
			updatePipeline_->GetRootIndex("gFreeList"), views->GetGPUDescriptorHandle(world.freeListUavIndex));
		commandList->SetComputeRootDescriptorTable(
			updatePipeline_->GetRootIndex("gCounters"), views->GetGPUDescriptorHandle(world.counterUavIndex));
		commandList->SetComputeRootConstantBufferView(
			updatePipeline_->GetRootIndex("GpuParticleConstants"), world.constantResources.GetGPUVirtualAddress(frameIndex));
		commandList->Dispatch(DispatchCount(world.desc.maximumParticleCount), 1u, 1u);
		const D3D12_RESOURCE_BARRIER updateBarriers[] = {
			MakeUavBarrier(world.particleResource.Get()),
			MakeUavBarrier(world.freeListResource.Get()),
			MakeUavBarrier(world.counterResource.Get()),
		};
		commandList->ResourceBarrier(_countof(updateBarriers), updateBarriers);
	}

	void GpuParticleDrawer3D::DispatchDraw(World& world) {
		auto* commandList = context_.directX.GetCommandList6();
		auto* views = context_.directX.GetResMgr().srvuav;
		commandList->SetGraphicsRootSignature(drawPipeline_->GetRootSignature());
		commandList->SetPipelineState(drawPipeline_->GetPipelineState());
		context_.cameraSys.Bind(commandList, drawPipeline_->GetRootIndex("gCamera"));
		commandList->SetGraphicsRootDescriptorTable(
			drawPipeline_->GetRootIndex("gParticles"), views->GetGPUDescriptorHandle(world.particleUavIndex));
		commandList->DispatchMesh(world.desc.maximumParticleCount, 1u, 1u);
	}

	void GpuParticleDrawer3D::CopyStatistics(World& world, uint32_t frameIndex) {
		auto* commandList = context_.directX.GetCommandList();
		D3D12_RESOURCE_BARRIER barrier = MakeTransitionBarrier(
			world.counterResource.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
		commandList->ResourceBarrier(1u, &barrier);
		commandList->CopyBufferRegion(
			world.readbackResources[frameIndex].Get(), 0u, world.counterResource.Get(), 0u, sizeof(CountersForGPU));
		barrier = MakeTransitionBarrier(
			world.counterResource.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandList->ResourceBarrier(1u, &barrier);
		world.hasReadback[frameIndex] = true;
	}

}
