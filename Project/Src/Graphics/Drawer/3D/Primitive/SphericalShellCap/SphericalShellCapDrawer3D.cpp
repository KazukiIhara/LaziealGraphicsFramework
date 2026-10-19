#include "SphericalShellCapDrawer3D.h"

#include <cassert>

#include "Graphics/Camera/System/Camera3DSystem.h"
#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/DirectX/ViewManagers/SRVUAVManager/SRVUAVManager.h"
#include "Graphics/Pipeline/Manager/PipelineManager.h"
#include "Graphics/Pipeline/Pipeline.h"

using namespace LGF;

namespace {
	constexpr UINT kSphericalShellCapMeshletCount = 9;
}

SphericalShellCapDrawer3D::SphericalShellCapDrawer3D(Graphics3DContext context) :
	BaseDrawer3D(context, kSphericalShellCapMeshletCount) {
}

SphericalShellCapDrawer3D::~SphericalShellCapDrawer3D() = default;

void SphericalShellCapDrawer3D::Initialize() {
	BaseDrawer::InitParam initParam{
		4096,
		sizeof(SphericalShellCap3DForGPU),
		reinterpret_cast<void**>(&instanceData_)
	};
	BaseDrawer::Initialize(initParam);

	SetPipeline(GetContext().pipelineMgr.GetPipeline("SphericalShellCap3D"));
	assert(GetPipeline() != nullptr);
}

void SphericalShellCapDrawer3D::EndFrame() {
	if (CheckIntstanceCount() == 0) {
		return;
	}

	ID3D12GraphicsCommandList6* commandList = GetContext().directX.GetCommandList6();
	commandList->SetGraphicsRootSignature(GetPipeline()->GetRootSignature());
	commandList->SetPipelineState(GetPipeline()->GetPipelineState());

	GetContext().cameraSys.Bind(commandList, GetPipeline()->GetRootIndex("gCamera"));
	commandList->SetGraphicsRootDescriptorTable(
		GetPipeline()->GetRootIndex("gSphericalShellCap"),
		GetContext().directX.GetResMgr().srvuav->GetGPUDescriptorHandle(instanceSrvIndex_));

	commandList->DispatchMesh(instanceCount_ * GetMeshletCountPerInstance(), 1, 1);
}

void SphericalShellCapDrawer3D::AddSphericalShellCap(
	const SphericalShellCap3DForGPU& data) {
	assert(currentIndex_ < numMaxInstance_);
	instanceData_[currentIndex_] = data;
	++currentIndex_;
}
