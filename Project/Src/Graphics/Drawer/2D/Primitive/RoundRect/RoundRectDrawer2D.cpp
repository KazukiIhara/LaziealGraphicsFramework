#include "RoundRectDrawer2D.h"

#include <cassert>

#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/DirectX/ViewManagers/SRVUAVManager/SRVUAVManager.h"
#include "Graphics/Pipeline/Pipeline.h"
#include "Graphics/Pipeline/Manager/PipelineManager.h"

using namespace LGF;

RoundRectDrawer2D::RoundRectDrawer2D(Graphics2DContext context) :
	BaseDrawer2D(context) {
}

RoundRectDrawer2D::~RoundRectDrawer2D() = default;

void RoundRectDrawer2D::Initialize() {
	BaseDrawer::InitParam initParam{
		4096,
		sizeof(RoundRect2DForGPU),
		reinterpret_cast<void**>(&instanceData_)
	};
	BaseDrawer::Initialize(initParam);

	SetPipeline(GetContext().pipelineMgr.GetPipeline("RoundRect2D"));
	assert(GetPipeline() != nullptr);
}

void RoundRectDrawer2D::EndFrame() {
	if (CheckIntstanceCount() == 0) {
		return;
	}

	ID3D12GraphicsCommandList6* commandList = GetContext().directX.GetCommandList6();
	commandList->SetGraphicsRootSignature(GetPipeline()->GetRootSignature());
	commandList->SetPipelineState(GetPipeline()->GetPipelineState());
	commandList->SetGraphicsRootDescriptorTable(
		GetPipeline()->GetRootIndex("gRoundRect"),
		GetContext().directX.GetResMgr().srvuav->GetGPUDescriptorHandle(instanceSrvIndex_));
	commandList->DispatchMesh(instanceCount_, 1, 1);
}

void RoundRectDrawer2D::AddRoundRect(const RoundRect2DForGPU& data) {
	assert(currentIndex_ < numMaxInstance_);
	instanceData_[currentIndex_] = data;
	++currentIndex_;
}
