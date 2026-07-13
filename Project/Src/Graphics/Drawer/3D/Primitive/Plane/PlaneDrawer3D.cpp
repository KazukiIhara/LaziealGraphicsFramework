#include "PlaneDrawer3D.h"

#include <cassert>

#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/Pipeline/Manager/PipelineManager.h"
#include "Graphics/Camera/System/Camera3DSystem.h"
#include "Graphics/DirectX/ViewManagers/SRVUAVManager/SRVUAVManager.h"

#include "Graphics/Pipeline/Pipeline.h"

using namespace LGF;

PlaneDrawer3D::PlaneDrawer3D(Graphics3DContext context) :
	BaseDrawer3D(context) {
}

PlaneDrawer3D::~PlaneDrawer3D() = default;

void PlaneDrawer3D::Initialize() {
	BaseDrawer::InitParam initParam{
		4096,
		sizeof(Plane3DForGPU),
		reinterpret_cast<void**>(&instanceData_)
	};
	BaseDrawer::Initialize(initParam);

	SetPipeline(GetContext().pipelineMgr.GetPipeline("Plane3D"));
	assert(GetPipeline() != nullptr);
}

void PlaneDrawer3D::EndFrame() {
	if (CheckIntstanceCount() == 0) {
		return;
	}

	ID3D12GraphicsCommandList6* commandList = GetContext().directX.GetCommandList6();

	commandList->SetGraphicsRootSignature(GetPipeline()->GetRootSignature());
	commandList->SetPipelineState(GetPipeline()->GetPipelineState());

	GetContext().cameraSys.Bind(commandList, GetPipeline()->GetRootIndex("gCamera"));

	commandList->SetGraphicsRootDescriptorTable(GetPipeline()->GetRootIndex("gPlane"), GetContext().directX.GetResMgr().srvuav->GetGPUDescriptorHandle(instanceSrvIndex_));

	commandList->DispatchMesh(instanceCount_, 1, 1);
}

void PlaneDrawer3D::AddPlane(const Plane3DForGPU& data) {
	assert(currentIndex_ < numMaxInstance_);
	instanceData_[currentIndex_] = data;
	++currentIndex_;
}
