#include "ModelDrawer3D.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <stdexcept>
#include <utility>

#include "Graphics/Camera/System/Camera3DSystem.h"
#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/DirectX/ResourceFactory/ResourceFactory.h"
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

}

ModelDrawer3D::ModelDrawer3D(Graphics3DContext context) :
	BaseDrawer3D(context) {
}

ModelDrawer3D::~ModelDrawer3D() {
	if (instanceResource_ && instanceData_) {
		instanceResource_->Unmap(0, nullptr);
	}
	if (drawConstantResource_ && drawConstantData_) {
		drawConstantResource_->Unmap(0, nullptr);
	}
}

void ModelDrawer3D::Initialize() {
	BaseDrawer::InitParam instanceInitParam{
		kMaxModelInstanceCount,
		sizeof(ModelInstanceData),
		reinterpret_cast<void**>(&instanceData_)
	};
	BaseDrawer::Initialize(instanceInitParam);

	drawConstantResource_ = GetContext().directX.GetResourceFactory()->CreateUploadBufferResource(
		static_cast<size_t>(kConstantBufferStride) * kMaxDispatchCount);
	if (!drawConstantResource_) {
		throw std::runtime_error("Model draw constant buffer allocation failed.");
	}

	void* mapped = nullptr;
	const HRESULT hr = drawConstantResource_->Map(0, nullptr, &mapped);
	if (FAILED(hr) || !mapped) {
		throw std::runtime_error("Model draw constant buffer mapping failed.");
	}
	drawConstantData_ = static_cast<std::byte*>(mapped);
	batches_.reserve(256);

	SetPipeline(GetContext().pipelineMgr.GetPipeline("Model3D"));
	assert(GetPipeline() != nullptr);
}

void ModelDrawer3D::EndFrame() {
	if (batches_.empty()) {
		return;
	}
	DrawBatches(GetPipeline(), nullptr);

	batches_.clear();
	batchIndices_.clear();
	queuedInstanceCount_ = 0;
}

void ModelDrawer3D::DrawShadow(ShadowMapSystem& shadowMap) {
	if (batches_.empty()) {
		return;
	}
	DrawBatches(GetContext().pipelineMgr.GetPipeline("ShadowModel3D"), &shadowMap);
}

void ModelDrawer3D::DrawBatches(Pipeline* pipeline, ShadowMapSystem* shadowMap) {
	assert(pipeline);

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

		for (const ModelMeshResource& mesh : batch.resource->meshes) {
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
				constants.vertexBufferIndex = mesh.vertexSrvIndex;
				constants.meshletBufferIndex = mesh.meshletSrvIndex;
				constants.meshletVertexIndexBufferIndex = mesh.meshletVertexIndexSrvIndex;
				constants.meshletPrimitiveBufferIndex = mesh.meshletPrimitiveSrvIndex;
				constants.meshletCullDataBufferIndex = mesh.meshletCullDataSrvIndex;
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
					drawConstantData_ + constantOffset,
					&constants,
					sizeof(constants));
				const D3D12_GPU_VIRTUAL_ADDRESS constantAddress =
					drawConstantResource_->GetGPUVirtualAddress() + constantOffset;
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

	const ModelResource* resourceKey = resource.get();
	const auto [batchIt, inserted] = batchIndices_.try_emplace(
		resourceKey,
		batches_.size());
	if (inserted) {
		batches_.push_back({ .resource = resource });
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
