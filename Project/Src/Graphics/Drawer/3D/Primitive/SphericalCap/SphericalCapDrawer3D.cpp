#include "SphericalCapDrawer3D.h"

#include <cassert>

#include "Graphics/Camera/System/Camera3DSystem.h"
#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/DirectX/ViewManagers/SRVUAVManager/SRVUAVManager.h"
#include "Graphics/Pipeline/Manager/PipelineManager.h"
#include "Graphics/Pipeline/Pipeline.h"

using namespace LGF;

namespace {
	constexpr UINT kSphericalCapMeshletCount = 4;
}

SphericalCapDrawer3D::SphericalCapDrawer3D(Graphics3DContext context) :
	BaseDrawer3D(context, kSphericalCapMeshletCount) {
}

SphericalCapDrawer3D::~SphericalCapDrawer3D() = default;

void SphericalCapDrawer3D::Initialize() {
	BaseDrawer::InitParam initParam{
		4096,
		sizeof(SphericalCap3DForGPU),
		reinterpret_cast<void**>(&instanceData_)
	};
	BaseDrawer::Initialize(initParam);

	SetPipeline(GetContext().pipelineMgr.GetPipeline("SphericalCap3D"));
	assert(GetPipeline() != nullptr);
}

void SphericalCapDrawer3D::EndFrame() {
	if (CheckIntstanceCount() == 0) {
		return;
	}

	ID3D12GraphicsCommandList6* commandList = GetContext().directX.GetCommandList6();
	commandList->SetGraphicsRootSignature(GetPipeline()->GetRootSignature());
	commandList->SetPipelineState(GetPipeline()->GetPipelineState());

	GetContext().cameraSys.Bind(commandList, GetPipeline()->GetRootIndex("gCamera"));
	commandList->SetGraphicsRootDescriptorTable(
		GetPipeline()->GetRootIndex("gSphericalCap"),
		GetContext().directX.GetResMgr().srvuav->GetGPUDescriptorHandle(instanceSrvIndex_));

	commandList->DispatchMesh(instanceCount_ * GetMeshletCountPerInstance(), 1, 1);
}

void SphericalCapDrawer3D::AddSphericalCap(const SphericalCap3DForGPU& data) {
	assert(currentIndex_ < numMaxInstance_);
	instanceData_[currentIndex_] = data;
	++currentIndex_;
}
