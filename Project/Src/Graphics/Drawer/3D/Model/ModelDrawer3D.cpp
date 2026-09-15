#include "ModelDrawer3D.h"

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
		Matrix4x4 worldMatrix;
		Matrix4x4 worldInverseTranspose;
		Vector4 baseColor;
		uint32_t vertexBufferIndex = UINT32_MAX;
		uint32_t meshletBufferIndex = UINT32_MAX;
		uint32_t meshletVertexIndexBufferIndex = UINT32_MAX;
		uint32_t meshletPrimitiveBufferIndex = UINT32_MAX;
		uint32_t meshletCullDataBufferIndex = UINT32_MAX;
		uint32_t baseColorTextureIndex = UINT32_MAX;
		uint32_t meshletCount = 0;
		uint32_t meshletIndexOffset = 0;
		uint32_t debugFlags = 0;
		float worldMaximumScale = 1.0f;
		uint32_t padding[2]{};
	};
	static_assert(sizeof(ModelDrawConstants) == 192);

	constexpr uint32_t kMeshletsPerAmplificationGroup = 32;
	constexpr uint32_t kDebugShowMeshletColors = 1u << 0;

}

ModelDrawer3D::ModelDrawer3D(Graphics3DContext context) :
	BaseDrawer3D(context) {
}

ModelDrawer3D::~ModelDrawer3D() {
	if (drawConstantResource_ && drawConstantData_) {
		drawConstantResource_->Unmap(0, nullptr);
	}
}

void ModelDrawer3D::Initialize() {
	drawConstantResource_ = GetContext().directX.GetResourceFactory()->CreateUploadBufferResource(
		static_cast<size_t>(kConstantBufferStride) * kMaxMeshDrawCount);
	if (!drawConstantResource_) {
		throw std::runtime_error("Model draw constant buffer allocation failed.");
	}

	void* mapped = nullptr;
	const HRESULT hr = drawConstantResource_->Map(0, nullptr, &mapped);
	if (FAILED(hr) || !mapped) {
		throw std::runtime_error("Model draw constant buffer mapping failed.");
	}
	drawConstantData_ = static_cast<std::byte*>(mapped);
	drawCommands_.reserve(kMaxMeshDrawCount);

	SetPipeline(GetContext().pipelineMgr.GetPipeline("Model3D"));
	assert(GetPipeline() != nullptr);
}

void ModelDrawer3D::EndFrame() {
	if (drawCommands_.empty()) {
		return;
	}

	ID3D12GraphicsCommandList6* commandList = GetContext().directX.GetCommandList6();
	commandList->SetGraphicsRootSignature(GetPipeline()->GetRootSignature());
	commandList->SetPipelineState(GetPipeline()->GetPipelineState());
	GetContext().cameraSys.Bind(commandList, GetPipeline()->GetRootIndex("gCamera"));
	GetContext().cameraSys.BindCulling(
		commandList,
		GetPipeline()->GetRootIndex("gCullingCamera"));

	uint32_t drawIndex = 0;
	for (const DrawCommand& command : drawCommands_) {
		for (const ModelMeshResource& mesh : command.resource->meshes) {
			ModelDrawConstants constants{};
			constants.worldMatrix = command.worldMatrix;
			constants.worldInverseTranspose = command.worldInverseTranspose;
			constants.baseColor = mesh.baseColor * command.color;
			constants.vertexBufferIndex = mesh.vertexSrvIndex;
			constants.meshletBufferIndex = mesh.meshletSrvIndex;
			constants.meshletVertexIndexBufferIndex = mesh.meshletVertexIndexSrvIndex;
			constants.meshletPrimitiveBufferIndex = mesh.meshletPrimitiveSrvIndex;
			constants.meshletCullDataBufferIndex = mesh.meshletCullDataSrvIndex;
			constants.baseColorTextureIndex = mesh.baseColorTextureSrvIndex;
			constants.meshletCount = mesh.meshletCount;
			constants.meshletIndexOffset = mesh.meshletIndexOffset;
			constants.debugFlags = command.showMeshletColors
				? kDebugShowMeshletColors
				: 0;
			constants.worldMaximumScale = command.worldMaximumScale;

			const size_t offset = static_cast<size_t>(drawIndex) * kConstantBufferStride;
			std::memcpy(drawConstantData_ + offset, &constants, sizeof(constants));
			commandList->SetGraphicsRootConstantBufferView(
				GetPipeline()->GetRootIndex("gModel"),
				drawConstantResource_->GetGPUVirtualAddress() + offset);
			commandList->SetGraphicsRootConstantBufferView(
				GetPipeline()->GetRootIndex("gCullingModel"),
				drawConstantResource_->GetGPUVirtualAddress() + offset);

			const uint32_t amplificationGroupCount =
				(mesh.meshletCount + kMeshletsPerAmplificationGroup - 1) /
				kMeshletsPerAmplificationGroup;
			commandList->DispatchMesh(amplificationGroupCount, 1, 1);
			++drawIndex;
		}
	}

	drawCommands_.clear();
	queuedMeshCount_ = 0;
}

void ModelDrawer3D::AddModel(
	std::shared_ptr<ModelResource> resource,
	const Matrix4x4& worldMatrix,
	const Matrix4x4& worldInverseTranspose,
	const Vector4& color,
	float worldMaximumScale,
	bool showMeshletColors) {

	if (!resource) {
		throw std::invalid_argument("Model resource must not be null.");
	}
	if (resource->meshes.size() > kMaxMeshDrawCount - queuedMeshCount_) {
		throw std::runtime_error("Too many model meshes were submitted in one frame.");
	}

	queuedMeshCount_ += static_cast<uint32_t>(resource->meshes.size());
	drawCommands_.push_back({
		.resource = std::move(resource),
		.worldMatrix = worldMatrix,
		.worldInverseTranspose = worldInverseTranspose,
		.color = color,
		.worldMaximumScale = worldMaximumScale,
		.showMeshletColors = showMeshletColors,
	});
}
