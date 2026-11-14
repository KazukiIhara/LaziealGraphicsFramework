#include "GroundDrawer3D.h"

#include <cassert>

#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/Pipeline/Manager/PipelineManager.h"
#include "Graphics/Camera/System/Camera3DSystem.h"
#include "Graphics/DirectX/ViewManagers/SRVUAVManager/SRVUAVManager.h"

#include "Graphics/Pipeline/Pipeline.h"

using namespace LGF;

GroundDrawer3D::GroundDrawer3D(Graphics3DContext context) :
	BaseDrawer3D(context) {
}

GroundDrawer3D::~GroundDrawer3D() = default;

void GroundDrawer3D::Initialize() {
	BaseDrawer::InitParam initParam{
		4096,
		sizeof(Ground3DForGPU),
		reinterpret_cast<void**>(&instanceData_)
	};
	BaseDrawer::Initialize(initParam);

	SetPipeline(GetContext().pipelineMgr.GetPipeline("Ground3D"));
	assert(GetPipeline() != nullptr);
}

void GroundDrawer3D::EndFrame() {
	const RenderPassInstanceRange range = GetCurrentRenderPassRange();
	if (range.count == 0u) {
		return;
	}

	ID3D12GraphicsCommandList6* commandList = GetContext().directX.GetCommandList6();

	commandList->SetGraphicsRootSignature(GetPipeline()->GetRootSignature());
	commandList->SetPipelineState(GetPipeline()->GetPipelineState());

	GetContext().cameraSys.Bind(commandList, GetPipeline()->GetRootIndex("gCamera"));
	BindDrawRange(GetPipeline());

	commandList->SetGraphicsRootDescriptorTable(GetPipeline()->GetRootIndex("gGround"), GetContext().directX.GetResMgr().srvuav->GetGPUDescriptorHandle(instanceSrvIndex_));

	commandList->DispatchMesh(range.count, 1, 1);
}

void GroundDrawer3D::AddGround(const Ground3DForGPU& data) {
	AddInstance(&data);
}
