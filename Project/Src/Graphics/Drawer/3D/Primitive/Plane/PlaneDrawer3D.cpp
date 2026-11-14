#include "PlaneDrawer3D.h"

#include <cassert>
#include <utility>

#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/Pipeline/Manager/PipelineManager.h"
#include "Graphics/Camera/System/Camera3DSystem.h"
#include "Graphics/DirectX/ViewManagers/SRVUAVManager/SRVUAVManager.h"

#include "Graphics/Pipeline/Pipeline.h"

using namespace LGF;

PlaneDrawer3D::PlaneDrawer3D(Graphics3DContext context, std::string pipelineName) :
	BaseDrawer3D(context),
	pipelineName_(std::move(pipelineName)) {
}

PlaneDrawer3D::~PlaneDrawer3D() = default;

void PlaneDrawer3D::Initialize() {
	BaseDrawer::InitParam initParam{
		4096,
		sizeof(Plane3DForGPU),
		reinterpret_cast<void**>(&instanceData_)
	};
	BaseDrawer::Initialize(initParam);

	SetPipeline(GetContext().pipelineMgr.GetPipeline(pipelineName_));
	assert(GetPipeline() != nullptr);
}

void PlaneDrawer3D::EndFrame() {
	const RenderPassInstanceRange range = GetCurrentRenderPassRange();
	if (range.count == 0u) {
		return;
	}

	ID3D12GraphicsCommandList6* commandList = GetContext().directX.GetCommandList6();

	commandList->SetGraphicsRootSignature(GetPipeline()->GetRootSignature());
	commandList->SetPipelineState(GetPipeline()->GetPipelineState());

	GetContext().cameraSys.Bind(commandList, GetPipeline()->GetRootIndex("gCamera"));
	BindDrawRange(GetPipeline());

	commandList->SetGraphicsRootDescriptorTable(GetPipeline()->GetRootIndex("gPlane"), GetContext().directX.GetResMgr().srvuav->GetGPUDescriptorHandle(instanceSrvIndex_));

	commandList->DispatchMesh(range.count, 1, 1);
}

void PlaneDrawer3D::AddPlane(const Plane3DForGPU& data) {
	AddInstance(&data);
}
