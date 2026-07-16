#include "SphereDrawer3D.h"

#include <cassert>

#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/Pipeline/Manager/PipelineManager.h"
#include "Graphics/Camera/System/Camera3DSystem.h"
#include "Graphics/DirectX/ViewManagers/SRVUAVManager/SRVUAVManager.h"

#include "Graphics/Pipeline/Pipeline.h"

using namespace LGF;

namespace {
	constexpr UINT kSphereMeshletCount = 4;
}

SphereDrawer3D::SphereDrawer3D(Graphics3DContext context) :
	BaseDrawer3D(context) {
}

SphereDrawer3D::~SphereDrawer3D() = default;

void SphereDrawer3D::Initialize() {
	BaseDrawer::InitParam initParam{
		4096,
		sizeof(Sphere3DForGPU),
		reinterpret_cast<void**>(&instanceData_)
	};
	BaseDrawer::Initialize(initParam);

	SetPipeline(GetContext().pipelineMgr.GetPipeline("Sphere3D"));
	assert(GetPipeline() != nullptr);
}

void SphereDrawer3D::EndFrame() {
	if (CheckIntstanceCount() == 0) {
		return;
	}

	ID3D12GraphicsCommandList6* commandList = GetContext().directX.GetCommandList6();

	commandList->SetGraphicsRootSignature(GetPipeline()->GetRootSignature());
	commandList->SetPipelineState(GetPipeline()->GetPipelineState());

	GetContext().cameraSys.Bind(commandList, GetPipeline()->GetRootIndex("gCamera"));

	commandList->SetGraphicsRootDescriptorTable(GetPipeline()->GetRootIndex("gSphere"), GetContext().directX.GetResMgr().srvuav->GetGPUDescriptorHandle(instanceSrvIndex_));

	commandList->DispatchMesh(instanceCount_ * kSphereMeshletCount, 1, 1);
}

void SphereDrawer3D::AddSphere(const Sphere3DForGPU& data) {
	assert(currentIndex_ < numMaxInstance_);
	instanceData_[currentIndex_] = data;
	++currentIndex_;
}
