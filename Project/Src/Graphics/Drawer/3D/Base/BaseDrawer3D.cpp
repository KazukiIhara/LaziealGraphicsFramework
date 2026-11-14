#include "BaseDrawer3D.h"

#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/DirectX/ViewManagers/SRVUAVManager/SRVUAVManager.h"
#include "Graphics/Pipeline/Pipeline.h"
#include "Graphics/Shadow/System/ShadowMapSystem.h"
#include "Graphics/Camera/System/Camera3DSystem.h"

using namespace LGF;

BaseDrawer3D::BaseDrawer3D(
	Graphics3DContext context,
	uint32_t meshletCountPerInstance) :
	BaseDrawer(context.directX, context.pipelineMgr),
	context_(context),
	meshletCountPerInstance_(meshletCountPerInstance) {
}

BaseDrawer3D::~BaseDrawer3D() = default;

Graphics3DContext BaseDrawer3D::GetContext() const {
	return context_;
}

uint32_t BaseDrawer3D::GetMeshletCountPerInstance() const {
	return meshletCountPerInstance_;
}

void BaseDrawer3D::DrawShadow(Pipeline* pipeline, ShadowMapSystem& shadowMap, const char* instanceBufferName) {
	const RenderPassInstanceRange range = GetRenderPassInstanceRange(0u);
	if (!pipeline || range.count == 0u) {
		return;
	}

	ID3D12GraphicsCommandList6* commandList = GetContext().directX.GetCommandList6();
	commandList->SetGraphicsRootSignature(pipeline->GetRootSignature());
	commandList->SetPipelineState(pipeline->GetPipelineState());
	shadowMap.BindCamera(commandList, pipeline->GetRootIndex("gCamera"));
	commandList->SetGraphicsRootConstantBufferView(
		pipeline->GetRootIndex("gDrawRange"),
		GetDrawRangeAddress(0u));
	commandList->SetGraphicsRootDescriptorTable(
		pipeline->GetRootIndex(instanceBufferName),
		GetContext().directX.GetResMgr().srvuav->GetGPUDescriptorHandle(instanceSrvIndex_));
	commandList->DispatchMesh(
		range.count * meshletCountPerInstance_,
		1,
		1);
}

void BaseDrawer3D::SetRecordingRenderPass(uint32_t renderPassIndex) {
	recordingRenderPassIndex_ = renderPassIndex;
}

void BaseDrawer3D::PrepareRenderPasses() {
	PrepareRenderPassInstances();
}

void BaseDrawer3D::AddInstance(const void* data) {
	BaseDrawer::AddInstance(data, recordingRenderPassIndex_);
}

RenderPassInstanceRange BaseDrawer3D::GetCurrentRenderPassRange() const {
	return GetRenderPassInstanceRange(GetContext().cameraSys.GetActiveRenderPass());
}

void BaseDrawer3D::BindDrawRange(Pipeline* pipeline) {
	GetContext().directX.GetCommandList()->SetGraphicsRootConstantBufferView(
		pipeline->GetRootIndex("gDrawRange"),
		GetDrawRangeAddress(GetContext().cameraSys.GetActiveRenderPass()));
}
