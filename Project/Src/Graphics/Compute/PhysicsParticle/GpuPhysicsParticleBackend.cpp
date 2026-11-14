#include "GpuPhysicsParticleBackend.h"

#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/DirectX/ResourceFactory/ResourceFactory.h"
#include "Graphics/DirectX/ViewManagers/SRVUAVManager/SRVUAVManager.h"
#include "Graphics/Camera/System/Camera3DSystem.h"
#include "Graphics/Pipeline/Manager/PipelineManager.h"
#include "Graphics/Pipeline/Pipeline.h"
#include "Graphics/Model/ModelResource.h"
#include "LGF/Graphics/Model.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <utility>

namespace {
	constexpr uint32_t kThreadCount = 64u;
	constexpr uint32_t kConstantStride = 256u;
	constexpr uint32_t kMaximumSphereDrawRequestCount = 64u;
	constexpr uint32_t kSphereMeshletCount = 4u;
	constexpr uint32_t kMaximumMeshDispatchCount = 65535u;
	constexpr uint32_t kMaximumParticlesPerSphereDispatch =
		kMaximumMeshDispatchCount / kSphereMeshletCount;
	constexpr uint32_t kMaximumModelDrawDispatchCount = 8192u;

	struct SphereDrawConstants {
		uint32_t renderGroupIndex{};
		uint32_t particleOffset{};
		uint32_t particleCount{};
		uint32_t padding{};
		LGF::Vector4 color{};
	};
	static_assert(sizeof(SphereDrawConstants) == 32u);

	struct ModelDrawConstants {
		LGF::Vector4 meshBaseColor{};
		uint32_t vertexBufferIndex = UINT32_MAX;
		uint32_t meshletBufferIndex = UINT32_MAX;
		uint32_t meshletVertexIndexBufferIndex = UINT32_MAX;
		uint32_t meshletPrimitiveBufferIndex = UINT32_MAX;
		uint32_t baseColorTextureIndex = UINT32_MAX;
		uint32_t normalTextureIndex = UINT32_MAX;
		uint32_t metallicRoughnessTextureIndex = UINT32_MAX;
		uint32_t occlusionTextureIndex = UINT32_MAX;
		float normalScale = 1.0f;
		float metallicFactor{};
		float roughnessFactor = 1.0f;
		float occlusionStrength = 1.0f;
		uint32_t renderGroupIndex{};
		uint32_t maximumParticleCount{};
		uint32_t meshletCount{};
		uint32_t baseWorkIndex{};
		LGF::Vector3 scale{ 1.0f, 1.0f, 1.0f };
		float padding{};
		LGF::Vector4 color{};
	};
	static_assert(sizeof(ModelDrawConstants) == 112u);

	uint32_t DispatchCount(uint32_t count) {
		return (count + kThreadCount - 1u) / kThreadCount;
	}

	uint32_t NextPowerOfTwo(uint32_t value) {
		value = std::max(value, 1u) - 1u;
		value |= value >> 1u;
		value |= value >> 2u;
		value |= value >> 4u;
		value |= value >> 8u;
		value |= value >> 16u;
		return value + 1u;
	}

	void AdvanceGeneration(uint32_t& generation) {
		if (++generation == 0u) {
			++generation;
		}
	}

	D3D12_RESOURCE_BARRIER UavBarrier(ID3D12Resource* resource) {
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		barrier.UAV.pResource = resource;
		return barrier;
	}

	D3D12_RESOURCE_BARRIER TransitionBarrier(
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

	GpuPhysicsParticleBackend::GpuPhysicsParticleBackend(
		DirectXCore& directX,
		PipelineManager& pipelineManager,
		Camera3DSystem& cameraSystem) :
		directX_(directX),
		pipelineManager_(pipelineManager),
		cameraSystem_(cameraSystem) {
	}

	GpuPhysicsParticleBackend::~GpuPhysicsParticleBackend() = default;

	bool GpuPhysicsParticleBackend::Initialize() {
		initializePipeline_ = pipelineManager_.GetPipeline("GpuPhysicsParticleInitialize");
		resetCountersPipeline_ = pipelineManager_.GetPipeline("GpuPhysicsParticleResetCounters");
		applyCommandsPipeline_ = pipelineManager_.GetPipeline("GpuPhysicsParticleApplyCommands");
		integratePipeline_ = pipelineManager_.GetPipeline("GpuPhysicsParticleIntegrate");
		clearGridPipeline_ = pipelineManager_.GetPipeline("GpuPhysicsParticleClearGrid");
		buildGridPipeline_ = pipelineManager_.GetPipeline("GpuPhysicsParticleBuildGrid");
		solvePositionsPipeline_ = pipelineManager_.GetPipeline("GpuPhysicsParticleSolvePositions");
		applyPositionsPipeline_ = pipelineManager_.GetPipeline("GpuPhysicsParticleApplyPositions");
		solveVelocitiesPipeline_ = pipelineManager_.GetPipeline("GpuPhysicsParticleSolveVelocities");
		applyVelocitiesPipeline_ = pipelineManager_.GetPipeline("GpuPhysicsParticleApplyVelocities");
		cleanupPipeline_ = pipelineManager_.GetPipeline("GpuPhysicsParticleCleanup");
		sphereDrawPipeline_ = pipelineManager_.GetPipeline("GpuPhysicsParticleSphere3D");
		modelDrawPipeline_ = pipelineManager_.GetPipeline("GpuPhysicsParticleModel3D");
		const Pipeline* pipelines[]{
			initializePipeline_, resetCountersPipeline_, applyCommandsPipeline_, integratePipeline_,
			clearGridPipeline_, buildGridPipeline_, solvePositionsPipeline_, applyPositionsPipeline_,
			solveVelocitiesPipeline_, applyVelocitiesPipeline_, cleanupPipeline_, sphereDrawPipeline_,
			modelDrawPipeline_,
		};
		for (const Pipeline* pipeline : pipelines) {
			if (!pipeline || !pipeline->IsValid()) {
				return false;
			}
		}
		return true;
	}

	void GpuPhysicsParticleBackend::BeginFrame() {
		const uint32_t frameIndex = directX_.GetCurrentFrameContextIndex();
		for (WorldSlot& slot : worlds_) {
			if (!slot.world) {
				continue;
			}
			Readback(*slot.world, frameIndex);
			ReadbackStates(*slot.world, frameIndex);
			slot.world->hasUpdate = false;
			slot.world->commands.clear();
			slot.world->commandRanges.clear();
			slot.world->colliders.clear();
			slot.world->sphereDrawRequests.clear();
			slot.world->modelDrawRequests.clear();
		}
	}

	void GpuPhysicsParticleBackend::DrawQueued() {
		for (WorldSlot& slot : worlds_) {
			if (slot.world && !slot.world->sphereDrawRequests.empty()) {
				DispatchSphereDraws(*slot.world);
			}
			if (slot.world && !slot.world->modelDrawRequests.empty()) {
				DispatchModelDraws(*slot.world);
			}
		}
	}

	void GpuPhysicsParticleBackend::EndFrame() {
		const uint32_t frameIndex = directX_.GetCurrentFrameContextIndex();
		for (WorldSlot& slot : worlds_) {
			if (!slot.world) {
				continue;
			}
			World& world = *slot.world;
			bool wroteGpuState = false;
			if (world.needsInitialization) {
				WriteFrameData(world, frameIndex);
				DispatchInitialize(world, frameIndex);
				world.needsInitialization = false;
				wroteGpuState = true;
			}
			if (world.hasUpdate) {
				WriteFrameData(world, frameIndex);
				DispatchSimulation(world, frameIndex);
				wroteGpuState = true;
			}
			if (wroteGpuState) {
				CopyReadback(world, frameIndex);
			}
			if (world.stateReadbackRequested) {
				CopyStateReadback(world, frameIndex);
				world.stateReadbackRequested = false;
			}
		}
	}

	void GpuPhysicsParticleBackend::Finalize() {
		if (!worlds_.empty()) {
			directX_.WaitIdle();
		}
		for (WorldSlot& slot : worlds_) {
			if (slot.world) {
				ReleaseWorldViews(*slot.world);
			}
		}
		worlds_.clear();
		freeWorldIndices_.clear();
	}

	GpuPhysicsParticleWorldHandle GpuPhysicsParticleBackend::CreateWorld(
		const GpuPhysicsParticleWorldDesc& desc) {
		if (desc.maximumParticleCount == 0u || desc.maximumColliderCount == 0u ||
			desc.maximumCommandCount == 0u || desc.substepCount == 0u ||
			desc.solverIterationCount == 0u ||
			!std::isfinite(desc.maximumParticleRadius) || desc.maximumParticleRadius <= 0.0f) {
			return {};
		}

		uint32_t index{};
		if (freeWorldIndices_.empty()) {
			index = static_cast<uint32_t>(worlds_.size());
			worlds_.emplace_back();
		} else {
			index = freeWorldIndices_.back();
			freeWorldIndices_.pop_back();
		}

		WorldSlot& slot = worlds_[index];
		slot.world = std::make_unique<World>();
		slot.world->desc = desc;
		slot.world->hashBucketCount = NextPowerOfTwo(desc.maximumParticleCount * 2u);
		if (!InitializeWorld(*slot.world)) {
			slot.world.reset();
			freeWorldIndices_.push_back(index);
			return {};
		}
		return { index, slot.generation };
	}

	bool GpuPhysicsParticleBackend::RemoveWorld(GpuPhysicsParticleWorldHandle handle) {
		World* world = FindWorld(handle);
		if (!world) {
			return false;
		}
		directX_.WaitIdle();
		ReleaseWorldViews(*world);
		WorldSlot& slot = worlds_[handle.index];
		slot.world.reset();
		AdvanceGeneration(slot.generation);
		freeWorldIndices_.push_back(handle.index);
		return true;
	}

	bool GpuPhysicsParticleBackend::ClearWorld(GpuPhysicsParticleWorldHandle handle) {
		World* world = FindWorld(handle);
		if (!world) {
			return false;
		}
		world->commands.clear();
		world->colliders.clear();
		world->retiredParticles.clear();
		world->latestStates.clear();
		world->latestStatistics = {};
		world->needsInitialization = true;
		return true;
	}

	bool GpuPhysicsParticleBackend::SetWorldSettings(
		GpuPhysicsParticleWorldHandle handle,
		const GpuPhysicsParticleWorldDesc& desc) {
		World* world = FindWorld(handle);
		if (!world || desc.maximumParticleCount != world->desc.maximumParticleCount ||
			desc.maximumColliderCount != world->desc.maximumColliderCount ||
			desc.maximumCommandCount != world->desc.maximumCommandCount ||
			desc.substepCount != world->desc.substepCount ||
			desc.maximumParticleRadius != world->desc.maximumParticleRadius ||
			desc.solverIterationCount == 0u) {
			return false;
		}
		world->desc = desc;
		return true;
	}

	bool GpuPhysicsParticleBackend::UpdateWorld(
		GpuPhysicsParticleWorldHandle handle,
		float deltaTime,
		std::span<const GpuPhysicsParticleCommand> commands,
		std::span<const GpuPhysicsCollider> colliders,
		uint32_t colliderCountPerSubstep) {
		World* world = FindWorld(handle);
		if (!world || !std::isfinite(deltaTime) || deltaTime <= 0.0f ||
			commands.size() > world->desc.maximumCommandCount ||
			colliderCountPerSubstep > world->desc.maximumColliderCount ||
			colliders.size() != static_cast<std::size_t>(colliderCountPerSubstep) * world->desc.substepCount) {
			return false;
		}
		world->deltaTime = deltaTime;
		world->commands.assign(commands.begin(), commands.end());
		std::stable_sort(world->commands.begin(), world->commands.end(),
			[](const GpuPhysicsParticleCommand& left, const GpuPhysicsParticleCommand& right) {
				return left.index < right.index;
			});
		world->commandRanges.clear();
		const uint32_t commandCount = static_cast<uint32_t>(world->commands.size());
		for (uint32_t first = 0u; first < commandCount;) {
			uint32_t end = first + 1u;
			while (end < commandCount &&
				world->commands[end].index == world->commands[first].index) {
				++end;
			}
			world->commandRanges.push_back({ first, end - first });
			first = end;
		}
		world->colliders.assign(colliders.begin(), colliders.end());
		world->colliderCountPerSubstep = colliderCountPerSubstep;
		world->hasUpdate = true;
		return true;
	}

	GpuPhysicsParticleStatistics GpuPhysicsParticleBackend::GetStatistics(
		GpuPhysicsParticleWorldHandle handle) const {
		const World* world = FindWorld(handle);
		return world ? world->latestStatistics : GpuPhysicsParticleStatistics{};
	}

	bool GpuPhysicsParticleBackend::ConsumeRetiredParticles(
		GpuPhysicsParticleWorldHandle handle,
		std::vector<GpuRetiredPhysicsParticle>& particles) {
		World* world = FindWorld(handle);
		if (!world) {
			return false;
		}
		particles = std::move(world->retiredParticles);
		world->retiredParticles.clear();
		return true;
	}

	bool GpuPhysicsParticleBackend::RequestStates(GpuPhysicsParticleWorldHandle handle) {
		World* world = FindWorld(handle);
		if (!world) {
			return false;
		}
		world->stateReadbackRequested = true;
		return true;
	}

	bool GpuPhysicsParticleBackend::GetLatestStates(
		GpuPhysicsParticleWorldHandle handle,
		std::vector<GpuPhysicsParticleState>& states,
		uint32_t& latencyFrames) const {
		const World* world = FindWorld(handle);
		if (!world || world->latestStates.empty()) {
			states.clear();
			latencyFrames = 0u;
			return false;
		}
		states = world->latestStates;
		latencyFrames = directX_.GetFrameContextCountInUse();
		return true;
	}

	bool GpuPhysicsParticleBackend::GetLatestState(
		GpuPhysicsParticleWorldHandle handle,
		uint32_t particleIndex,
		GpuPhysicsParticleState& state,
		uint32_t& latencyFrames) const {
		const World* world = FindWorld(handle);
		if (!world || particleIndex >= world->latestStates.size()) {
			latencyFrames = 0u;
			return false;
		}
		state = world->latestStates[particleIndex];
		latencyFrames = directX_.GetFrameContextCountInUse();
		return true;
	}

	bool GpuPhysicsParticleBackend::DrawSpheres(
		GpuPhysicsParticleWorldHandle handle,
		uint32_t renderGroupIndex,
		const Vector4& color) {
		World* world = FindWorld(handle);
		if (!world || renderGroupIndex == UINT32_MAX ||
			world->sphereDrawRequests.size() >= kMaximumSphereDrawRequestCount ||
			!std::isfinite(color.x) || !std::isfinite(color.y) ||
			!std::isfinite(color.z) || !std::isfinite(color.w)) {
			return false;
		}
		world->sphereDrawRequests.push_back({ renderGroupIndex, color });
		return true;
	}

	bool GpuPhysicsParticleBackend::DrawModel(
		GpuPhysicsParticleWorldHandle handle,
		uint32_t renderGroupIndex,
		const Model& model,
		const Vector3& scale,
		const Vector4& color) {
		World* world = FindWorld(handle);
		if (!world || renderGroupIndex == UINT32_MAX || !model.resource_ ||
			model.resource_->rig ||
			world->modelDrawRequests.size() >= kMaximumSphereDrawRequestCount ||
			!std::isfinite(scale.x) || !std::isfinite(scale.y) || !std::isfinite(scale.z) ||
			scale.x <= 0.0f || scale.y <= 0.0f || scale.z <= 0.0f ||
			!std::isfinite(color.x) || !std::isfinite(color.y) ||
			!std::isfinite(color.z) || !std::isfinite(color.w)) {
			return false;
		}
		world->modelDrawRequests.push_back({
			.renderGroupIndex = renderGroupIndex,
			.resource = model.resource_,
			.scale = scale,
			.color = color,
		});
		return true;
	}

	GpuPhysicsParticleBackend::World* GpuPhysicsParticleBackend::FindWorld(
		GpuPhysicsParticleWorldHandle handle) {
		return const_cast<World*>(std::as_const(*this).FindWorld(handle));
	}

	const GpuPhysicsParticleBackend::World* GpuPhysicsParticleBackend::FindWorld(
		GpuPhysicsParticleWorldHandle handle) const {
		if (!handle.IsValid() || handle.index >= worlds_.size()) {
			return nullptr;
		}
		const WorldSlot& slot = worlds_[handle.index];
		return slot.world && slot.generation == handle.generation ? slot.world.get() : nullptr;
	}

	bool GpuPhysicsParticleBackend::InitializeWorld(World& world) {
		auto* factory = directX_.GetResourceFactory();
		auto* views = directX_.GetResMgr().srvuav;
		const uint32_t particleCount = world.desc.maximumParticleCount;
		const uint32_t viewCount = 7u + kFrameContextCount * 3u;
		if (!views->CanAllocate(viewCount)) {
			return false;
		}

		world.particleResource = factory->CreateUAVBufferResource(sizeof(ParticleForGPU) * particleCount);
		world.correctionResource = factory->CreateUAVBufferResource(sizeof(Vector4) * particleCount);
		world.bucketHeadResource = factory->CreateUAVBufferResource(sizeof(uint32_t) * world.hashBucketCount);
		world.particleCellResource = factory->CreateUAVBufferResource(sizeof(int32_t) * 4u * particleCount);
		world.particleNextResource = factory->CreateUAVBufferResource(sizeof(uint32_t) * particleCount);
		world.counterResource = factory->CreateUAVBufferResource(sizeof(CountersForGPU));
		world.retiredResource = factory->CreateUAVBufferResource(sizeof(GpuRetiredPhysicsParticle) * particleCount);
		if (!world.particleResource || !world.correctionResource || !world.bucketHeadResource ||
			!world.particleCellResource || !world.particleNextResource || !world.counterResource ||
			!world.retiredResource) {
			return false;
		}

		world.particleUavIndex = views->Allocate();
		world.correctionUavIndex = views->Allocate();
		world.bucketHeadUavIndex = views->Allocate();
		world.particleCellUavIndex = views->Allocate();
		world.particleNextUavIndex = views->Allocate();
		world.counterUavIndex = views->Allocate();
		world.retiredUavIndex = views->Allocate();
		views->CreateUavStructuredBuffer(world.particleUavIndex, world.particleResource.Get(), particleCount, sizeof(ParticleForGPU));
		views->CreateUavStructuredBuffer(world.correctionUavIndex, world.correctionResource.Get(), particleCount, sizeof(Vector4));
		views->CreateUavStructuredBuffer(world.bucketHeadUavIndex, world.bucketHeadResource.Get(), world.hashBucketCount, sizeof(uint32_t));
		views->CreateUavStructuredBuffer(world.particleCellUavIndex, world.particleCellResource.Get(), particleCount, sizeof(int32_t) * 4u);
		views->CreateUavStructuredBuffer(world.particleNextUavIndex, world.particleNextResource.Get(), particleCount, sizeof(uint32_t));
		views->CreateUavStructuredBuffer(world.counterUavIndex, world.counterResource.Get(), 1u, sizeof(CountersForGPU));
		views->CreateUavStructuredBuffer(world.retiredUavIndex, world.retiredResource.Get(), particleCount, sizeof(GpuRetiredPhysicsParticle));

		const std::size_t commandBytes = sizeof(GpuPhysicsParticleCommand) * world.desc.maximumCommandCount;
		const std::size_t commandRangeBytes = sizeof(CommandRangeForGPU) * world.desc.maximumParticleCount;
		const std::size_t colliderBytes = sizeof(GpuPhysicsCollider) * world.desc.maximumColliderCount * world.desc.substepCount;
		const std::size_t constantBytes = kConstantStride * world.desc.substepCount;
		const std::size_t drawChunkCount =
			(world.desc.maximumParticleCount + kMaximumParticlesPerSphereDispatch - 1u) /
			kMaximumParticlesPerSphereDispatch;
		const std::size_t drawConstantBytes = kConstantStride *
			kMaximumSphereDrawRequestCount * drawChunkCount;
		if (!world.commandResources.Initialize(*factory, commandBytes) ||
			!world.commandRangeResources.Initialize(*factory, commandRangeBytes) ||
			!world.colliderResources.Initialize(*factory, colliderBytes) ||
			!world.constantResources.Initialize(*factory, constantBytes) ||
			!world.drawConstantResources.Initialize(*factory, drawConstantBytes) ||
			!world.modelDrawConstantResources.Initialize(
				*factory,
				kConstantStride * kMaximumModelDrawDispatchCount)) {
			return false;
		}
		for (uint32_t i = 0u; i < kFrameContextCount; ++i) {
			world.commandSrvIndices[i] = views->Allocate();
			world.commandRangeSrvIndices[i] = views->Allocate();
			world.colliderSrvIndices[i] = views->Allocate();
			views->CreateSrvStructuredBuffer(
				world.commandSrvIndices[i],
				world.commandResources.GetResource(i),
				world.desc.maximumCommandCount,
				sizeof(GpuPhysicsParticleCommand));
			views->CreateSrvStructuredBuffer(
				world.commandRangeSrvIndices[i],
				world.commandRangeResources.GetResource(i),
				world.desc.maximumParticleCount,
				sizeof(CommandRangeForGPU));
			views->CreateSrvStructuredBuffer(
				world.colliderSrvIndices[i],
				world.colliderResources.GetResource(i),
				world.desc.maximumColliderCount * world.desc.substepCount,
				sizeof(GpuPhysicsCollider));
			const std::size_t readbackBytes = sizeof(CountersForGPU) + sizeof(GpuRetiredPhysicsParticle) * particleCount;
			world.readbackResources[i] = factory->CreateReadbackBufferResource(readbackBytes);
			world.stateReadbackResources[i] = factory->CreateReadbackBufferResource(
				sizeof(ParticleForGPU) * particleCount);
			if (!world.readbackResources[i] || !world.stateReadbackResources[i]) {
				return false;
			}
		}
		return true;
	}

	void GpuPhysicsParticleBackend::ReadbackStates(World& world, uint32_t frameIndex) {
		if (!world.hasStateReadback[frameIndex]) {
			return;
		}
		void* mapped = nullptr;
		const std::size_t size = sizeof(ParticleForGPU) * world.desc.maximumParticleCount;
		const D3D12_RANGE range{ 0u, size };
		if (FAILED(world.stateReadbackResources[frameIndex]->Map(0u, &range, &mapped)) || !mapped) {
			return;
		}
		world.latestStates.resize(world.desc.maximumParticleCount);
		std::memcpy(world.latestStates.data(), mapped, size);
		const D3D12_RANGE writtenRange{};
		world.stateReadbackResources[frameIndex]->Unmap(0u, &writtenRange);
		world.hasStateReadback[frameIndex] = false;
	}

	void GpuPhysicsParticleBackend::ReleaseWorldViews(World& world) {
		auto* views = directX_.GetResMgr().srvuav;
		views->Release(world.particleUavIndex);
		views->Release(world.correctionUavIndex);
		views->Release(world.bucketHeadUavIndex);
		views->Release(world.particleCellUavIndex);
		views->Release(world.particleNextUavIndex);
		views->Release(world.counterUavIndex);
		views->Release(world.retiredUavIndex);
		for (uint32_t i = 0u; i < kFrameContextCount; ++i) {
			views->Release(world.commandSrvIndices[i]);
			views->Release(world.commandRangeSrvIndices[i]);
			views->Release(world.colliderSrvIndices[i]);
		}
	}

	void GpuPhysicsParticleBackend::Readback(World& world, uint32_t frameIndex) {
		if (!world.hasReadback[frameIndex]) {
			return;
		}
		void* mapped = nullptr;
		const std::size_t maximumBytes = sizeof(CountersForGPU) +
			sizeof(GpuRetiredPhysicsParticle) * world.desc.maximumParticleCount;
		const D3D12_RANGE range{ 0u, maximumBytes };
		if (FAILED(world.readbackResources[frameIndex]->Map(0u, &range, &mapped)) || !mapped) {
			return;
		}
		CountersForGPU counters{};
		std::memcpy(&counters, mapped, sizeof(counters));
		const uint32_t retiredCount = std::min(counters.retiredCount, world.desc.maximumParticleCount);
		const auto* retired = reinterpret_cast<const GpuRetiredPhysicsParticle*>(
			static_cast<const std::byte*>(mapped) + sizeof(CountersForGPU));
		world.retiredParticles.insert(world.retiredParticles.end(), retired, retired + retiredCount);
		const D3D12_RANGE writtenRange{};
		world.readbackResources[frameIndex]->Unmap(0u, &writtenRange);
		world.hasReadback[frameIndex] = false;
		world.latestStatistics = {
			.particleCount = counters.aliveCount,
			.candidatePairCount = counters.candidatePairCount,
			.retiredParticleCount = counters.retiredCount,
			.latencyFrames = directX_.GetFrameContextCountInUse(),
			.isAvailable = true,
		};
	}

	void GpuPhysicsParticleBackend::WriteFrameData(World& world, uint32_t frameIndex) {
		if (!world.commands.empty()) {
			std::memcpy(world.commandResources.GetMappedData(frameIndex), world.commands.data(),
				world.commands.size() * sizeof(GpuPhysicsParticleCommand));
		}
		if (!world.commandRanges.empty()) {
			std::memcpy(world.commandRangeResources.GetMappedData(frameIndex), world.commandRanges.data(),
				world.commandRanges.size() * sizeof(CommandRangeForGPU));
		}
		if (!world.colliders.empty()) {
			std::memcpy(world.colliderResources.GetMappedData(frameIndex), world.colliders.data(),
				world.colliders.size() * sizeof(GpuPhysicsCollider));
		}

		const float substepTime = world.deltaTime / static_cast<float>(world.desc.substepCount);
		for (uint32_t substep = 0u; substep < world.desc.substepCount; ++substep) {
			auto* constants = reinterpret_cast<ConstantsForGPU*>(
				world.constantResources.GetMappedData(frameIndex) + kConstantStride * substep);
			*constants = {
				.worldMinimum = world.desc.worldMinimum,
				.deltaTime = substepTime,
				.worldMaximum = world.desc.worldMaximum,
				.damping = std::exp(-world.desc.linearDamping * substepTime),
				.gravity = world.desc.gravity,
				.maximumCorrection = world.desc.maximumDepenetrationVelocity * substepTime /
					static_cast<float>(world.desc.solverIterationCount),
				.maximumParticleCount = world.desc.maximumParticleCount,
				.hashBucketCount = world.hashBucketCount,
				.colliderCount = world.colliderCountPerSubstep,
				.commandCount = static_cast<uint32_t>(world.commands.size()),
				.substepIndex = substep,
				.solverIterationCount = world.desc.solverIterationCount,
				.commandRangeCount = static_cast<uint32_t>(world.commandRanges.size()),
				.cellSize = world.desc.maximumParticleRadius * 2.0f,
				.penetrationSlop = world.desc.penetrationSlop,
				.positionCorrectionRate = world.desc.positionCorrectionRate,
				.bounceVelocityThreshold = world.desc.bounceVelocityThreshold,
			};
		}
	}

	void GpuPhysicsParticleBackend::DispatchInitialize(World& world, uint32_t frameIndex) {
		auto* commandList = directX_.GetCommandList();
		auto* views = directX_.GetResMgr().srvuav;
		commandList->SetComputeRootSignature(initializePipeline_->GetRootSignature());
		commandList->SetPipelineState(initializePipeline_->GetPipelineState());
		commandList->SetComputeRootDescriptorTable(initializePipeline_->GetRootIndex("gParticles"), views->GetGPUDescriptorHandle(world.particleUavIndex));
		commandList->SetComputeRootDescriptorTable(initializePipeline_->GetRootIndex("gBucketHeads"), views->GetGPUDescriptorHandle(world.bucketHeadUavIndex));
		commandList->SetComputeRootDescriptorTable(initializePipeline_->GetRootIndex("gCounters"), views->GetGPUDescriptorHandle(world.counterUavIndex));
		commandList->SetComputeRootConstantBufferView(
			initializePipeline_->GetRootIndex("GpuPhysicsParticleConstants"),
			world.constantResources.GetGPUVirtualAddress(frameIndex));
		commandList->Dispatch(DispatchCount(std::max(world.desc.maximumParticleCount, world.hashBucketCount)), 1u, 1u);
		const D3D12_RESOURCE_BARRIER barriers[]{
			UavBarrier(world.particleResource.Get()),
			UavBarrier(world.bucketHeadResource.Get()),
			UavBarrier(world.counterResource.Get()),
		};
		commandList->ResourceBarrier(_countof(barriers), barriers);
	}

	void GpuPhysicsParticleBackend::DispatchSimulation(World& world, uint32_t frameIndex) {
		auto* commandList = directX_.GetCommandList();
		auto* views = directX_.GetResMgr().srvuav;
		const auto bindConstants = [&](Pipeline& pipeline, uint32_t substep) {
			commandList->SetComputeRootConstantBufferView(
				pipeline.GetRootIndex("GpuPhysicsParticleConstants"),
				world.constantResources.GetGPUVirtualAddress(frameIndex) + kConstantStride * substep);
		};

		commandList->SetComputeRootSignature(resetCountersPipeline_->GetRootSignature());
		commandList->SetPipelineState(resetCountersPipeline_->GetPipelineState());
		commandList->SetComputeRootDescriptorTable(resetCountersPipeline_->GetRootIndex("gCounters"), views->GetGPUDescriptorHandle(world.counterUavIndex));
		commandList->Dispatch(1u, 1u, 1u);
		D3D12_RESOURCE_BARRIER barrier = UavBarrier(world.counterResource.Get());
		commandList->ResourceBarrier(1u, &barrier);

		if (!world.commands.empty()) {
			commandList->SetComputeRootSignature(applyCommandsPipeline_->GetRootSignature());
			commandList->SetPipelineState(applyCommandsPipeline_->GetPipelineState());
			commandList->SetComputeRootDescriptorTable(applyCommandsPipeline_->GetRootIndex("gParticles"), views->GetGPUDescriptorHandle(world.particleUavIndex));
			commandList->SetComputeRootDescriptorTable(
				applyCommandsPipeline_->GetRootIndex("gCommandRanges"),
				views->GetGPUDescriptorHandle(world.commandRangeSrvIndices[frameIndex]));
			commandList->SetComputeRootDescriptorTable(
				applyCommandsPipeline_->GetRootIndex("gCommands"),
				views->GetGPUDescriptorHandle(world.commandSrvIndices[frameIndex]));
			commandList->SetComputeRootDescriptorTable(applyCommandsPipeline_->GetRootIndex("gCounters"), views->GetGPUDescriptorHandle(world.counterUavIndex));
			bindConstants(*applyCommandsPipeline_, 0u);
			commandList->Dispatch(DispatchCount(static_cast<uint32_t>(world.commandRanges.size())), 1u, 1u);
			const D3D12_RESOURCE_BARRIER barriers[]{ UavBarrier(world.particleResource.Get()), UavBarrier(world.counterResource.Get()) };
			commandList->ResourceBarrier(_countof(barriers), barriers);
		}

		for (uint32_t substep = 0u; substep < world.desc.substepCount; ++substep) {
			DispatchParticlePass(*integratePipeline_, world, frameIndex, substep);

			commandList->SetComputeRootSignature(clearGridPipeline_->GetRootSignature());
			commandList->SetPipelineState(clearGridPipeline_->GetPipelineState());
			commandList->SetComputeRootDescriptorTable(clearGridPipeline_->GetRootIndex("gBucketHeads"), views->GetGPUDescriptorHandle(world.bucketHeadUavIndex));
			bindConstants(*clearGridPipeline_, substep);
			commandList->Dispatch(DispatchCount(world.hashBucketCount), 1u, 1u);
			barrier = UavBarrier(world.bucketHeadResource.Get());
			commandList->ResourceBarrier(1u, &barrier);

			commandList->SetComputeRootSignature(buildGridPipeline_->GetRootSignature());
			commandList->SetPipelineState(buildGridPipeline_->GetPipelineState());
			commandList->SetComputeRootDescriptorTable(buildGridPipeline_->GetRootIndex("gParticles"), views->GetGPUDescriptorHandle(world.particleUavIndex));
			commandList->SetComputeRootDescriptorTable(buildGridPipeline_->GetRootIndex("gBucketHeads"), views->GetGPUDescriptorHandle(world.bucketHeadUavIndex));
			commandList->SetComputeRootDescriptorTable(buildGridPipeline_->GetRootIndex("gParticleCells"), views->GetGPUDescriptorHandle(world.particleCellUavIndex));
			commandList->SetComputeRootDescriptorTable(buildGridPipeline_->GetRootIndex("gParticleNext"), views->GetGPUDescriptorHandle(world.particleNextUavIndex));
			bindConstants(*buildGridPipeline_, substep);
			commandList->Dispatch(DispatchCount(world.desc.maximumParticleCount), 1u, 1u);
			const D3D12_RESOURCE_BARRIER gridBarriers[]{
				UavBarrier(world.bucketHeadResource.Get()),
				UavBarrier(world.particleCellResource.Get()),
				UavBarrier(world.particleNextResource.Get()),
			};
			commandList->ResourceBarrier(_countof(gridBarriers), gridBarriers);

			for (uint32_t iteration = 0u; iteration < world.desc.solverIterationCount; ++iteration) {
				DispatchParticlePass(*solvePositionsPipeline_, world, frameIndex, substep);
				DispatchParticlePass(*applyPositionsPipeline_, world, frameIndex, substep);
			}
			DispatchParticlePass(*solveVelocitiesPipeline_, world, frameIndex, substep);
			DispatchParticlePass(*applyVelocitiesPipeline_, world, frameIndex, substep);
		}
		DispatchParticlePass(*cleanupPipeline_, world, frameIndex, world.desc.substepCount - 1u);
	}

	void GpuPhysicsParticleBackend::DispatchParticlePass(
		Pipeline& pipeline,
		World& world,
		uint32_t frameIndex,
		uint32_t substepIndex) {
		auto* commandList = directX_.GetCommandList();
		auto* views = directX_.GetResMgr().srvuav;
		commandList->SetComputeRootSignature(pipeline.GetRootSignature());
		commandList->SetPipelineState(pipeline.GetPipelineState());
		const std::string& name = pipeline.GetName();
		commandList->SetComputeRootDescriptorTable(pipeline.GetRootIndex("gParticles"), views->GetGPUDescriptorHandle(world.particleUavIndex));
		if (name == "GpuPhysicsParticleSolvePositions" || name == "GpuPhysicsParticleSolveVelocities") {
			commandList->SetComputeRootDescriptorTable(pipeline.GetRootIndex("gCorrections"), views->GetGPUDescriptorHandle(world.correctionUavIndex));
			commandList->SetComputeRootDescriptorTable(pipeline.GetRootIndex("gBucketHeads"), views->GetGPUDescriptorHandle(world.bucketHeadUavIndex));
			commandList->SetComputeRootDescriptorTable(pipeline.GetRootIndex("gParticleCells"), views->GetGPUDescriptorHandle(world.particleCellUavIndex));
			commandList->SetComputeRootDescriptorTable(pipeline.GetRootIndex("gParticleNext"), views->GetGPUDescriptorHandle(world.particleNextUavIndex));
			commandList->SetComputeRootDescriptorTable(pipeline.GetRootIndex("gColliders"), views->GetGPUDescriptorHandle(world.colliderSrvIndices[frameIndex]));
			if (name == "GpuPhysicsParticleSolvePositions") {
				commandList->SetComputeRootDescriptorTable(pipeline.GetRootIndex("gCounters"), views->GetGPUDescriptorHandle(world.counterUavIndex));
			}
		} else if (name == "GpuPhysicsParticleApplyPositions" || name == "GpuPhysicsParticleApplyVelocities") {
			commandList->SetComputeRootDescriptorTable(pipeline.GetRootIndex("gCorrections"), views->GetGPUDescriptorHandle(world.correctionUavIndex));
		} else if (name == "GpuPhysicsParticleCleanup") {
			commandList->SetComputeRootDescriptorTable(pipeline.GetRootIndex("gCounters"), views->GetGPUDescriptorHandle(world.counterUavIndex));
			commandList->SetComputeRootDescriptorTable(pipeline.GetRootIndex("gRetiredParticles"), views->GetGPUDescriptorHandle(world.retiredUavIndex));
		}
		commandList->SetComputeRootConstantBufferView(
			pipeline.GetRootIndex("GpuPhysicsParticleConstants"),
			world.constantResources.GetGPUVirtualAddress(frameIndex) + kConstantStride * substepIndex);
		commandList->Dispatch(DispatchCount(world.desc.maximumParticleCount), 1u, 1u);
		const D3D12_RESOURCE_BARRIER barriers[]{
			UavBarrier(world.particleResource.Get()), UavBarrier(world.correctionResource.Get()),
			UavBarrier(world.counterResource.Get()), UavBarrier(world.retiredResource.Get()),
		};
		commandList->ResourceBarrier(_countof(barriers), barriers);
	}

	void GpuPhysicsParticleBackend::DispatchSphereDraws(World& world) {
		auto* commandList = directX_.GetCommandList6();
		auto* views = directX_.GetResMgr().srvuav;
		const uint32_t frameIndex = directX_.GetCurrentFrameContextIndex();
		auto* mappedConstants = reinterpret_cast<std::byte*>(
			world.drawConstantResources.GetMappedData(frameIndex));
		uint32_t constantIndex = 0u;
		commandList->SetGraphicsRootSignature(sphereDrawPipeline_->GetRootSignature());
		commandList->SetPipelineState(sphereDrawPipeline_->GetPipelineState());
		cameraSystem_.Bind(commandList, sphereDrawPipeline_->GetRootIndex("gCamera"));
		commandList->SetGraphicsRootDescriptorTable(
			sphereDrawPipeline_->GetRootIndex("gParticles"),
			views->GetGPUDescriptorHandle(world.particleUavIndex));
		for (const World::SphereDrawRequest& request : world.sphereDrawRequests) {
			for (uint32_t particleOffset = 0u;
				particleOffset < world.desc.maximumParticleCount;
				particleOffset += kMaximumParticlesPerSphereDispatch) {
				const uint32_t particleCount = std::min(
					kMaximumParticlesPerSphereDispatch,
					world.desc.maximumParticleCount - particleOffset);
				const std::size_t constantOffset =
					static_cast<std::size_t>(constantIndex++) * kConstantStride;
				auto* constants = reinterpret_cast<SphereDrawConstants*>(
					mappedConstants + constantOffset);
				*constants = {
					.renderGroupIndex = request.renderGroupIndex,
					.particleOffset = particleOffset,
					.particleCount = particleCount,
					.color = request.color,
				};
				commandList->SetGraphicsRootConstantBufferView(
					sphereDrawPipeline_->GetRootIndex("GpuPhysicsParticleSphereConstants"),
					world.drawConstantResources.GetGPUVirtualAddress(frameIndex) + constantOffset);
				commandList->DispatchMesh(particleCount * kSphereMeshletCount, 1u, 1u);
			}
		}
	}

	void GpuPhysicsParticleBackend::DispatchModelDraws(World& world) {
		auto* commandList = directX_.GetCommandList6();
		auto* views = directX_.GetResMgr().srvuav;
		const uint32_t frameIndex = directX_.GetCurrentFrameContextIndex();
		auto* mappedConstants = reinterpret_cast<std::byte*>(
			world.modelDrawConstantResources.GetMappedData(frameIndex));
		uint32_t dispatchIndex = 0u;
		commandList->SetGraphicsRootSignature(modelDrawPipeline_->GetRootSignature());
		commandList->SetPipelineState(modelDrawPipeline_->GetPipelineState());
		cameraSystem_.Bind(commandList, modelDrawPipeline_->GetRootIndex("gCamera"));
		commandList->SetGraphicsRootDescriptorTable(
			modelDrawPipeline_->GetRootIndex("gParticles"),
			views->GetGPUDescriptorHandle(world.particleUavIndex));
		for (const World::ModelDrawRequest& request : world.modelDrawRequests) {
			for (const ModelMeshResource& mesh : request.resource->meshes) {
				const uint64_t workCount =
					static_cast<uint64_t>(world.desc.maximumParticleCount) * mesh.meshletCount;
				for (uint64_t baseWorkIndex = 0u; baseWorkIndex < workCount;
					baseWorkIndex += kMaximumMeshDispatchCount) {
					if (dispatchIndex >= kMaximumModelDrawDispatchCount) {
						return;
					}
					const uint32_t dispatchCount = static_cast<uint32_t>(std::min<uint64_t>(
						kMaximumMeshDispatchCount, workCount - baseWorkIndex));
					const std::size_t constantOffset =
						static_cast<std::size_t>(dispatchIndex++) * kConstantStride;
					auto* constants = reinterpret_cast<ModelDrawConstants*>(
						mappedConstants + constantOffset);
					*constants = {
						.meshBaseColor = mesh.baseColor,
						.vertexBufferIndex = mesh.vertexSrvIndex,
						.meshletBufferIndex = mesh.meshletSrvIndex,
						.meshletVertexIndexBufferIndex = mesh.meshletVertexIndexSrvIndex,
						.meshletPrimitiveBufferIndex = mesh.meshletPrimitiveSrvIndex,
						.baseColorTextureIndex = mesh.baseColorTextureSrvIndex,
						.normalTextureIndex = mesh.normalTextureSrvIndex,
						.metallicRoughnessTextureIndex = mesh.metallicRoughnessTextureSrvIndex,
						.occlusionTextureIndex = mesh.occlusionTextureSrvIndex,
						.normalScale = mesh.normalScale,
						.metallicFactor = mesh.metallicFactor,
						.roughnessFactor = mesh.roughnessFactor,
						.occlusionStrength = mesh.occlusionStrength,
						.renderGroupIndex = request.renderGroupIndex,
						.maximumParticleCount = world.desc.maximumParticleCount,
						.meshletCount = mesh.meshletCount,
						.baseWorkIndex = static_cast<uint32_t>(baseWorkIndex),
						.scale = request.scale,
						.color = request.color,
					};
					commandList->SetGraphicsRootConstantBufferView(
						modelDrawPipeline_->GetRootIndex("GpuPhysicsParticleModelConstants"),
						world.modelDrawConstantResources.GetGPUVirtualAddress(frameIndex) + constantOffset);
					commandList->DispatchMesh(dispatchCount, 1u, 1u);
				}
			}
		}
	}

	void GpuPhysicsParticleBackend::CopyReadback(World& world, uint32_t frameIndex) {
		auto* commandList = directX_.GetCommandList();
		D3D12_RESOURCE_BARRIER barriers[]{
			TransitionBarrier(world.counterResource.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE),
			TransitionBarrier(world.retiredResource.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE),
		};
		commandList->ResourceBarrier(_countof(barriers), barriers);
		commandList->CopyBufferRegion(world.readbackResources[frameIndex].Get(), 0u, world.counterResource.Get(), 0u, sizeof(CountersForGPU));
		commandList->CopyBufferRegion(
			world.readbackResources[frameIndex].Get(), sizeof(CountersForGPU),
			world.retiredResource.Get(), 0u,
			sizeof(GpuRetiredPhysicsParticle) * world.desc.maximumParticleCount);
		barriers[0] = TransitionBarrier(world.counterResource.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		barriers[1] = TransitionBarrier(world.retiredResource.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandList->ResourceBarrier(_countof(barriers), barriers);
		world.hasReadback[frameIndex] = true;
	}

	void GpuPhysicsParticleBackend::CopyStateReadback(World& world, uint32_t frameIndex) {
		auto* commandList = directX_.GetCommandList();
		D3D12_RESOURCE_BARRIER barrier = TransitionBarrier(
			world.particleResource.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			D3D12_RESOURCE_STATE_COPY_SOURCE);
		commandList->ResourceBarrier(1u, &barrier);
		commandList->CopyBufferRegion(
			world.stateReadbackResources[frameIndex].Get(),
			0u,
			world.particleResource.Get(),
			0u,
			sizeof(ParticleForGPU) * world.desc.maximumParticleCount);
		barrier = TransitionBarrier(
			world.particleResource.Get(),
			D3D12_RESOURCE_STATE_COPY_SOURCE,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandList->ResourceBarrier(1u, &barrier);
		world.hasStateReadback[frameIndex] = true;
	}

}
