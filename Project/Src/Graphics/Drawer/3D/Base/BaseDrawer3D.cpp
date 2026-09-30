#include "BaseDrawer3D.h"

#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/DirectX/ViewManagers/SRVUAVManager/SRVUAVManager.h"
#include "Graphics/Pipeline/Pipeline.h"
#include "Graphics/Shadow/System/ShadowMapSystem.h"

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
	if (!pipeline || currentIndex_ == 0) {
		return;
	}

	ID3D12GraphicsCommandList6* commandList = GetContext().directX.GetCommandList6();
	commandList->SetGraphicsRootSignature(pipeline->GetRootSignature());
	commandList->SetPipelineState(pipeline->GetPipelineState());
	shadowMap.BindCamera(commandList, pipeline->GetRootIndex("gCamera"));
	commandList->SetGraphicsRootDescriptorTable(
		pipeline->GetRootIndex(instanceBufferName),
		GetContext().directX.GetResMgr().srvuav->GetGPUDescriptorHandle(instanceSrvIndex_));
	commandList->DispatchMesh(
		currentIndex_ * meshletCountPerInstance_,
		1,
		1);
}
