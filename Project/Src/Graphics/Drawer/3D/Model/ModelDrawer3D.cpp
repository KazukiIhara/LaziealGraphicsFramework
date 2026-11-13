#include "ModelDrawer3D.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <stdexcept>
#include <utility>

#include "Graphics/Camera/System/Camera3DSystem.h"
#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/DirectX/ResourceFactory/ResourceFactory.h"
#include "Graphics/DirectX/ViewManagers/SRVUAVManager/SRVUAVManager.h"
#include "Graphics/Model/ModelResource.h"
#include "Graphics/Pipeline/Manager/PipelineManager.h"
#include "Graphics/Pipeline/Pipeline.h"
#include "Graphics/Shadow/System/ShadowMapSystem.h"

using namespace LGF;

namespace {

	struct ModelDrawConstants {
		Vector4 meshBaseColor;
		uint32_t vertexBufferIndex = UINT32_MAX;
		uint32_t meshletBufferIndex = UINT32_MAX;
		uint32_t meshletVertexIndexBufferIndex = UINT32_MAX;
		uint32_t meshletPrimitiveBufferIndex = UINT32_MAX;
		uint32_t meshletCullDataBufferIndex = UINT32_MAX;
		uint32_t baseColorTextureIndex = UINT32_MAX;
		uint32_t normalTextureIndex = UINT32_MAX;
		float normalScale = 1.0f;
		uint32_t metallicRoughnessTextureIndex = UINT32_MAX;
		uint32_t occlusionTextureIndex = UINT32_MAX;
		float metallicFactor = 1.0f;
		float roughnessFactor = 1.0f;
		float occlusionStrength = 1.0f;
		uint32_t instanceBufferIndex = UINT32_MAX;
		uint32_t instanceOffset = 0;
		uint32_t instanceCount = 0;
		uint32_t meshletCount = 0;
		uint32_t meshletIndexOffset = 0;
		uint32_t baseWorkIndexLow = 0;
		uint32_t baseWorkIndexHigh = 0;
		uint32_t padding[2]{};
	};
	static_assert(sizeof(ModelDrawConstants) == 104);

	constexpr uint32_t kMeshletsPerAmplificationGroup = 32;
	constexpr uint32_t kMaxAmplificationGroupCount = 65535;
	constexpr uint32_t kDebugShowMeshletColors = 1u << 0;
	constexpr uint32_t kDisableNormalMap = 1u << 1;
	constexpr uint32_t kUnlit = 1u << 2;

	struct SkinningConstants {
		uint32_t vertexCount = 0;
		uint32_t padding[3]{};
	};

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

size_t ModelDrawer3D::ModelBatchKeyHash::operator()(const ModelBatchKey& key) const {
	return std::hash<const void*>{}(key.resource) ^
		(std::hash<const void*>{}(key.animator) << 1);
}

ModelDrawer3D::ModelDrawer3D(Graphics3DContext context) :
	BaseDrawer3D(context) {
}

ModelDrawer3D::~ModelDrawer3D() {
}

void ModelDrawer3D::Initialize() {
	BaseDrawer::InitParam instanceInitParam{
		kMaxModelInstanceCount,
		sizeof(ModelInstanceData),
		reinterpret_cast<void**>(&instanceData_)
	};
	BaseDrawer::Initialize(instanceInitParam);

	if (!drawConstantResources_.Initialize(
		*GetContext().directX.GetResourceFactory(),
		static_cast<size_t>(kConstantBufferStride) * kMaxDispatchCount)) {
		throw std::runtime_error("Model draw constant buffer allocation failed.");
	}
	if (!skinningConstantResources_.Initialize(
		*GetContext().directX.GetResourceFactory(),
		static_cast<size_t>(kConstantBufferStride) * kMaxDispatchCount)) {
		throw std::runtime_error("Model skinning constant buffer allocation failed.");
	}

	batches_.reserve(256);

	SetPipeline(GetContext().pipelineMgr.GetPipeline("Model3D"));
	assert(GetPipeline() != nullptr);
}

void ModelDrawer3D::BeginFrame() {
	BaseDrawer::BeginFrame();
	skinningPrepared_ = false;
}

void ModelDrawer3D::EndFrame() {
	if (batches_.empty()) {
		return;
	}
	PrepareSkinning();
	DrawBatches(GetPipeline(), nullptr);

	batches_.clear();
	batchIndices_.clear();
	queuedInstanceCount_ = 0;
}

void ModelDrawer3D::DrawShadow(ShadowMapSystem& shadowMap) {
	if (batches_.empty()) {
		return;
	}
	PrepareSkinning();
	DrawBatches(GetContext().pipelineMgr.GetPipeline("ShadowModel3D"), &shadowMap);
}

void ModelDrawer3D::PrepareSkinning() {
	if (skinningPrepared_) {
		return;
	}
	skinningPrepared_ = true;
	Pipeline* pipeline = GetContext().pipelineMgr.GetPipeline("SkinningModel3D");
	assert(pipeline);
	auto& directX = GetContext().directX;
	auto* views = directX.GetResMgr().srvuav;
	auto* factory = directX.GetResourceFactory();
	const uint32_t frameIndex = directX.GetCurrentFrameContextIndex();
	ID3D12GraphicsCommandList6* commandList = directX.GetCommandList6();
	std::byte* constantData = skinningConstantResources_.GetMappedData(frameIndex);
	uint32_t dispatchIndex = 0;
	std::unordered_map<const ModelAnimatorResource*, bool> prepared;

	for (ModelBatch& batch : batches_) {
		if (!batch.animator || prepared.contains(batch.animator.get())) {
			continue;
		}
		prepared.emplace(batch.animator.get(), true);
		ModelAnimatorResource& animator = *batch.animator;
		if (!animator.gpuResourcesInitialized) {
			const uint32_t descriptorCount = kFrameContextCount +
				static_cast<uint32_t>(animator.meshes.size()) * 2u;
			if (!views->CanAllocate(descriptorCount)) {
				throw std::runtime_error("Model skinning descriptor heap is full.");
			}
			const size_t paletteSize = animator.palette.size() * sizeof(ModelSkinningPaletteEntry);
			if (!animator.paletteResources.Initialize(*factory, paletteSize)) {
				throw std::runtime_error("Model skinning palette allocation failed.");
			}
			for (uint32_t i = 0; i < kFrameContextCount; ++i) {
				animator.paletteSrvIndices[i] = views->Allocate();
				views->CreateSrvStructuredBuffer(animator.paletteSrvIndices[i],
					animator.paletteResources.GetResource(i),
					static_cast<uint32_t>(animator.palette.size()), sizeof(ModelSkinningPaletteEntry));
			}
			for (size_t meshIndex = 0; meshIndex < animator.meshes.size(); ++meshIndex) {
				auto& output = animator.meshes[meshIndex];
				const auto& mesh = animator.model->meshes[meshIndex];
				output.vertexResource = factory->CreateUAVBufferResource(
					static_cast<size_t>(mesh.vertexCount) * sizeof(ModelVertex));
				output.vertexSrvIndex = views->Allocate();
				output.vertexUavIndex = views->Allocate();
				views->CreateSrvStructuredBuffer(output.vertexSrvIndex, output.vertexResource.Get(), mesh.vertexCount, sizeof(ModelVertex));
				views->CreateUavStructuredBuffer(output.vertexUavIndex, output.vertexResource.Get(), mesh.vertexCount, sizeof(ModelVertex));
			}
			animator.gpuResourcesInitialized = true;
		}

		std::memcpy(animator.paletteResources.GetMappedData(frameIndex),
			animator.palette.data(), animator.palette.size() * sizeof(ModelSkinningPaletteEntry));
		commandList->SetComputeRootSignature(pipeline->GetRootSignature());
		commandList->SetPipelineState(pipeline->GetPipelineState());
		for (size_t meshIndex = 0; meshIndex < animator.meshes.size(); ++meshIndex) {
			if (dispatchIndex >= kMaxDispatchCount) {
				throw std::runtime_error("Too many model skinning dispatches were submitted in one frame.");
			}
			const auto& source = animator.model->meshes[meshIndex];
			auto& output = animator.meshes[meshIndex];
			const auto transition = MakeTransitionBarrier(output.vertexResource.Get(), output.state, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			commandList->ResourceBarrier(1, &transition);
			output.state = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
			commandList->SetComputeRootDescriptorTable(
				pipeline->GetRootIndex("gPalette"),
				views->GetGPUDescriptorHandle(animator.paletteSrvIndices[frameIndex]));
			commandList->SetComputeRootDescriptorTable(
				pipeline->GetRootIndex("gInputVertices"),
				views->GetGPUDescriptorHandle(source.vertexSrvIndex));
			commandList->SetComputeRootDescriptorTable(
				pipeline->GetRootIndex("gInfluences"),
				views->GetGPUDescriptorHandle(source.influenceSrvIndex));
			commandList->SetComputeRootDescriptorTable(
				pipeline->GetRootIndex("gOutputVertices"),
				views->GetGPUDescriptorHandle(output.vertexUavIndex));
			SkinningConstants constants{ .vertexCount = source.vertexCount };
			const size_t constantOffset = static_cast<size_t>(dispatchIndex) * kConstantBufferStride;
			std::memcpy(constantData + constantOffset, &constants, sizeof(constants));
			commandList->SetComputeRootConstantBufferView(pipeline->GetRootIndex("gSkinning"),
				skinningConstantResources_.GetGPUVirtualAddress(frameIndex) + constantOffset);
			commandList->Dispatch((source.vertexCount + 255u) / 256u, 1, 1);
			D3D12_RESOURCE_BARRIER barriers[2]{};
			barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
			barriers[0].UAV.pResource = output.vertexResource.Get();
			barriers[1] = MakeTransitionBarrier(output.vertexResource.Get(), output.state, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
			commandList->ResourceBarrier(2, barriers);
			output.state = D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
			++dispatchIndex;
		}
	}
}

void ModelDrawer3D::DrawBatches(Pipeline* pipeline, ShadowMapSystem* shadowMap) {
	assert(pipeline);
	const uint32_t frameIndex = GetContext().directX.GetCurrentFrameContextIndex();
	std::byte* drawConstantData = drawConstantResources_.GetMappedData(frameIndex);

	ID3D12GraphicsCommandList6* commandList = GetContext().directX.GetCommandList6();
	commandList->SetGraphicsRootSignature(pipeline->GetRootSignature());
	commandList->SetPipelineState(pipeline->GetPipelineState());
	if (shadowMap) {
		shadowMap->BindCamera(commandList, pipeline->GetRootIndex("gCamera"));
		shadowMap->BindCamera(commandList, pipeline->GetRootIndex("gCullingCamera"));
	} else {
		GetContext().cameraSys.Bind(commandList, pipeline->GetRootIndex("gCamera"));
		GetContext().cameraSys.BindCulling(commandList, pipeline->GetRootIndex("gCullingCamera"));
	}

	uint32_t instanceOffset = 0;
	uint32_t dispatchIndex = 0;
	for (const ModelBatch& batch : batches_) {
		const uint32_t instanceCount = static_cast<uint32_t>(batch.instances.size());
		std::memcpy(
			instanceData_ + instanceOffset,
			batch.instances.data(),
			batch.instances.size() * sizeof(ModelInstanceData));

		for (size_t meshIndex = 0; meshIndex < batch.resource->meshes.size(); ++meshIndex) {
			const ModelMeshResource& mesh = batch.resource->meshes[meshIndex];
			const uint64_t workCount =
				static_cast<uint64_t>(instanceCount) * mesh.meshletCount;
			uint64_t baseWorkIndex = 0;

			while (baseWorkIndex < workCount) {
				if (dispatchIndex >= kMaxDispatchCount) {
					throw std::runtime_error("Too many model mesh dispatches were submitted in one frame.");
				}

				const uint64_t remainingWorkCount = workCount - baseWorkIndex;
				const uint64_t remainingGroupCount =
					(remainingWorkCount + kMeshletsPerAmplificationGroup - 1) /
					kMeshletsPerAmplificationGroup;
				const uint32_t amplificationGroupCount = static_cast<uint32_t>(
					std::min<uint64_t>(remainingGroupCount, kMaxAmplificationGroupCount));

				ModelDrawConstants constants{};
				constants.meshBaseColor = mesh.baseColor;
				constants.vertexBufferIndex = batch.animator
					? batch.animator->meshes[meshIndex].vertexSrvIndex
					: mesh.vertexSrvIndex;
				constants.meshletBufferIndex = mesh.meshletSrvIndex;
				constants.meshletVertexIndexBufferIndex = mesh.meshletVertexIndexSrvIndex;
				constants.meshletPrimitiveBufferIndex = mesh.meshletPrimitiveSrvIndex;
				constants.meshletCullDataBufferIndex = batch.animator ? UINT32_MAX : mesh.meshletCullDataSrvIndex;
				constants.baseColorTextureIndex = mesh.baseColorTextureSrvIndex;
				constants.normalTextureIndex = mesh.normalTextureSrvIndex;
				constants.normalScale = mesh.normalScale;
				constants.metallicRoughnessTextureIndex = mesh.metallicRoughnessTextureSrvIndex;
				constants.occlusionTextureIndex = mesh.occlusionTextureSrvIndex;
				constants.metallicFactor = mesh.metallicFactor;
				constants.roughnessFactor = mesh.roughnessFactor;
				constants.occlusionStrength = mesh.occlusionStrength;
				constants.instanceBufferIndex = instanceSrvIndex_;
				constants.instanceOffset = instanceOffset;
				constants.instanceCount = instanceCount;
				constants.meshletCount = mesh.meshletCount;
				constants.meshletIndexOffset = mesh.meshletIndexOffset;
				constants.baseWorkIndexLow = static_cast<uint32_t>(baseWorkIndex);
				constants.baseWorkIndexHigh = static_cast<uint32_t>(baseWorkIndex >> 32);

				const size_t constantOffset =
					static_cast<size_t>(dispatchIndex) * kConstantBufferStride;
				std::memcpy(
					drawConstantData + constantOffset,
					&constants,
					sizeof(constants));
				const D3D12_GPU_VIRTUAL_ADDRESS constantAddress =
					drawConstantResources_.GetGPUVirtualAddress(frameIndex) + constantOffset;
				commandList->SetGraphicsRootConstantBufferView(
					pipeline->GetRootIndex("gModel"),
					constantAddress);
				commandList->SetGraphicsRootConstantBufferView(
					pipeline->GetRootIndex("gCullingModel"),
					constantAddress);
				commandList->DispatchMesh(amplificationGroupCount, 1, 1);

				baseWorkIndex +=
					static_cast<uint64_t>(amplificationGroupCount) *
					kMeshletsPerAmplificationGroup;
				++dispatchIndex;
			}
		}
		instanceOffset += instanceCount;
	}

}

void ModelDrawer3D::AddModel(
	std::shared_ptr<ModelResource> resource,
	std::shared_ptr<ModelAnimatorResource> animator,
	const Matrix4x4& worldMatrix,
	const Matrix4x4& worldInverseTranspose,
	const Vector4& color,
	float worldMaximumScale,
	float worldOrientationSign,
	bool showMeshletColors,
	bool enableNormalMap,
	float normalScale,
	float metallicScale,
	float roughnessScale,
	float occlusionStrength,
	bool unlit) {

	if (!resource) {
		throw std::invalid_argument("Model resource must not be null.");
	}
	if (queuedInstanceCount_ >= kMaxModelInstanceCount) {
		throw std::runtime_error("Too many model instances were submitted in one frame.");
	}

	if (animator && animator->model.get() != resource.get()) {
		throw std::invalid_argument("Model animator belongs to a different resource.");
	}
	const ModelBatchKey resourceKey{ resource.get(), animator.get() };
	const auto [batchIt, inserted] = batchIndices_.try_emplace(
		resourceKey,
		batches_.size());
	if (inserted) {
		batches_.push_back({ .resource = resource, .animator = animator });
	}

	ModelBatch& batch = batches_[batchIt->second];
	uint32_t drawFlags = showMeshletColors ? kDebugShowMeshletColors : 0;
	if (!enableNormalMap) {
		drawFlags |= kDisableNormalMap;
	}
	if (unlit) {
		drawFlags |= kUnlit;
	}
	batch.instances.push_back({
		.worldMatrix = worldMatrix,
		.worldInverseTranspose = worldInverseTranspose,
		.color = color,
		.worldMaximumScale = worldMaximumScale,
		.drawFlags = drawFlags,
		.worldOrientationSign = worldOrientationSign,
		.normalScale = normalScale,
		.metallicScale = std::max(metallicScale, 0.0f),
		.roughnessScale = std::max(roughnessScale, 0.0f),
		.occlusionStrength = std::clamp(occlusionStrength, 0.0f, 1.0f),
	});
	++queuedInstanceCount_;
}
