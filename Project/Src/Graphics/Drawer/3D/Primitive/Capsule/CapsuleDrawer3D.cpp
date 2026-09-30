#include "CapsuleDrawer3D.h"

#include <cassert>

#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/Pipeline/Manager/PipelineManager.h"
#include "Graphics/Camera/System/Camera3DSystem.h"
#include "Graphics/DirectX/ViewManagers/SRVUAVManager/SRVUAVManager.h"

#include "Graphics/Pipeline/Pipeline.h"

using namespace LGF;

namespace {
	constexpr UINT kCapsuleMeshletCount = 4;
}

CapsuleDrawer3D::CapsuleDrawer3D(Graphics3DContext context) :
	BaseDrawer3D(context, kCapsuleMeshletCount) {
}

CapsuleDrawer3D::~CapsuleDrawer3D() = default;

void CapsuleDrawer3D::Initialize() {
	BaseDrawer::InitParam initParam{
		4096,
		sizeof(Capsule3DForGPU),
		reinterpret_cast<void**>(&instanceData_)
	};
	BaseDrawer::Initialize(initParam);

	SetPipeline(GetContext().pipelineMgr.GetPipeline("Capsule3D"));
	assert(GetPipeline() != nullptr);
}

void CapsuleDrawer3D::EndFrame() {
	if (CheckIntstanceCount() == 0) {
		return;
	}

	ID3D12GraphicsCommandList6* commandList = GetContext().directX.GetCommandList6();

	commandList->SetGraphicsRootSignature(GetPipeline()->GetRootSignature());
	commandList->SetPipelineState(GetPipeline()->GetPipelineState());

	GetContext().cameraSys.Bind(commandList, GetPipeline()->GetRootIndex("gCamera"));

	commandList->SetGraphicsRootDescriptorTable(GetPipeline()->GetRootIndex("gCapsule"), GetContext().directX.GetResMgr().srvuav->GetGPUDescriptorHandle(instanceSrvIndex_));

	commandList->DispatchMesh(
		instanceCount_ * GetMeshletCountPerInstance(),
		1,
		1);
}

void CapsuleDrawer3D::AddCapsule(const Capsule3DForGPU& data) {
	assert(currentIndex_ < numMaxInstance_);
	instanceData_[currentIndex_] = data;
	++currentIndex_;
}
