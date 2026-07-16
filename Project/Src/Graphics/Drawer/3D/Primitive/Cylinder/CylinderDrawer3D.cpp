#include "CylinderDrawer3D.h"

#include <cassert>

#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/Pipeline/Manager/PipelineManager.h"
#include "Graphics/Camera/System/Camera3DSystem.h"
#include "Graphics/DirectX/ViewManagers/SRVUAVManager/SRVUAVManager.h"

#include "Graphics/Pipeline/Pipeline.h"

using namespace LGF;

CylinderDrawer3D::CylinderDrawer3D(Graphics3DContext context) :
	BaseDrawer3D(context) {
}

CylinderDrawer3D::~CylinderDrawer3D() = default;

void CylinderDrawer3D::Initialize() {
	BaseDrawer::InitParam initParam{
		4096,
		sizeof(Cylinder3DForGPU),
		reinterpret_cast<void**>(&instanceData_)
	};
	BaseDrawer::Initialize(initParam);

	SetPipeline(GetContext().pipelineMgr.GetPipeline("Cylinder3D"));
	assert(GetPipeline() != nullptr);
}

void CylinderDrawer3D::EndFrame() {
	if (CheckIntstanceCount() == 0) {
		return;
	}

	ID3D12GraphicsCommandList6* commandList = GetContext().directX.GetCommandList6();

	commandList->SetGraphicsRootSignature(GetPipeline()->GetRootSignature());
	commandList->SetPipelineState(GetPipeline()->GetPipelineState());

	GetContext().cameraSys.Bind(commandList, GetPipeline()->GetRootIndex("gCamera"));

	commandList->SetGraphicsRootDescriptorTable(GetPipeline()->GetRootIndex("gCylinder"), GetContext().directX.GetResMgr().srvuav->GetGPUDescriptorHandle(instanceSrvIndex_));

	commandList->DispatchMesh(instanceCount_, 1, 1);
}

void CylinderDrawer3D::AddCylinder(const Cylinder3DForGPU& data) {
	assert(currentIndex_ < numMaxInstance_);
	instanceData_[currentIndex_] = data;
	++currentIndex_;
}
