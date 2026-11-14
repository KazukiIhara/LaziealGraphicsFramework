#include "LineDrawer3D.h"

#include <cassert>

#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/Pipeline/Manager/PipelineManager.h"
#include "Graphics/Camera/System/Camera3DSystem.h"
#include "Graphics/DirectX/ViewManagers/SRVUAVManager/SRVUAVManager.h"

#include "Graphics/Pipeline/Pipeline.h"

using namespace LGF;

LineDrawer3D::LineDrawer3D(Graphics3DContext context) :
	BaseDrawer3D(context) {
}

LineDrawer3D::~LineDrawer3D() = default;

void LineDrawer3D::Initialize() {
	BaseDrawer::InitParam initParam{
		4096,
		sizeof(Line3DForGPU),
		reinterpret_cast<void**>(&instanceData_)
	};
	BaseDrawer::Initialize(initParam);

	SetPipeline(GetContext().pipelineMgr.GetPipeline("Line3D"));
	assert(GetPipeline() != nullptr);
}

void LineDrawer3D::EndFrame() {
	const RenderPassInstanceRange range = GetCurrentRenderPassRange();
	if (range.count == 0u) {
		return;
	}

	ID3D12GraphicsCommandList6* commandList = GetContext().directX.GetCommandList6();

	commandList->SetGraphicsRootSignature(GetPipeline()->GetRootSignature());
	commandList->SetPipelineState(GetPipeline()->GetPipelineState());

	GetContext().cameraSys.Bind(commandList, GetPipeline()->GetRootIndex("gCamera"));
	BindDrawRange(GetPipeline());

	commandList->SetGraphicsRootDescriptorTable(GetPipeline()->GetRootIndex("gLine"), GetContext().directX.GetResMgr().srvuav->GetGPUDescriptorHandle(instanceSrvIndex_));

	commandList->DispatchMesh(range.count, 1, 1);
}

void LineDrawer3D::AddLine(const Line3DForGPU& data) {
	AddInstance(&data);
}
