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
		uint32_t instanceBufferIndex = UINT32_MAX;
		uint32_t instanceOffset = 0;
		uint32_t instanceCount = 0;
		uint32_t meshletCount = 0;
		uint32_t meshletIndexOffset = 0;
		uint32_t baseWorkIndexLow = 0;
		uint32_t baseWorkIndexHigh = 0;
		uint32_t padding = 0;
	};
	static_assert(sizeof(ModelDrawConstants) == 80);

	constexpr uint32_t kMeshletsPerAmplificationGroup = 32;
	constexpr uint32_t kMaxAmplificationGroupCount = 65535;
	constexpr uint32_t kDebugShowMeshletColors = 1u << 0;
	constexpr uint32_t kDisableNormalMap = 1u << 1;

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

	ID3D12GraphicsCommandList6* commandList = GetContext().directX.GetCommandList6();
	commandList->SetGraphicsRootSignature(GetPipeline()->GetRootSignature());
	commandList->SetPipelineState(GetPipeline()->GetPipelineState());
	GetContext().cameraSys.Bind(commandList, GetPipeline()->GetRootIndex("gCamera"));
	GetContext().cameraSys.BindCulling(
		commandList,
		GetPipeline()->GetRootIndex("gCullingCamera"));

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
					GetPipeline()->GetRootIndex("gModel"),
					constantAddress);
				commandList->SetGraphicsRootConstantBufferView(
					GetPipeline()->GetRootIndex("gCullingModel"),
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

	batches_.clear();
	batchIndices_.clear();
	queuedInstanceCount_ = 0;
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
	float normalScale) {

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
	batch.instances.push_back({
		.worldMatrix = worldMatrix,
		.worldInverseTranspose = worldInverseTranspose,
		.color = color,
		.worldMaximumScale = worldMaximumScale,
		.drawFlags = drawFlags,
		.worldOrientationSign = worldOrientationSign,
		.normalScale = normalScale,
	});
	++queuedInstanceCount_;
}
