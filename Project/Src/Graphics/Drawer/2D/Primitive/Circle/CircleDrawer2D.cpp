#include "CircleDrawer2D.h"

#include <cassert>

#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/DirectX/ViewManagers/SRVUAVManager/SRVUAVManager.h"
#include "Graphics/Pipeline/Pipeline.h"
#include "Graphics/Pipeline/Manager/PipelineManager.h"

using namespace LGF;

CircleDrawer2D::CircleDrawer2D(Graphics2DContext context) :
	BaseDrawer2D(context) {
}

CircleDrawer2D::~CircleDrawer2D() = default;

void CircleDrawer2D::Initialize() {
	BaseDrawer::InitParam initParam{
		4096,
		sizeof(Circle2DForGPU),
		reinterpret_cast<void**>(&instanceData_)
	};
	BaseDrawer::Initialize(initParam);

	SetPipeline(GetContext().pipelineMgr.GetPipeline("Circle2D"));
	assert(GetPipeline() != nullptr);
}

void CircleDrawer2D::EndFrame() {
	if (CheckIntstanceCount() == 0) {
		return;
	}

	ID3D12GraphicsCommandList6* commandList = GetContext().directX.GetCommandList6();
	commandList->SetGraphicsRootSignature(GetPipeline()->GetRootSignature());
	commandList->SetPipelineState(GetPipeline()->GetPipelineState());
	commandList->SetGraphicsRootDescriptorTable(
		GetPipeline()->GetRootIndex("gCircle"),
		GetContext().directX.GetResMgr().srvuav->GetGPUDescriptorHandle(instanceSrvIndex_));
	commandList->DispatchMesh(instanceCount_, 1, 1);
}

void CircleDrawer2D::AddCircle(const Circle2DForGPU& data) {
	assert(currentIndex_ < numMaxInstance_);
	instanceData_[currentIndex_] = data;
	++currentIndex_;
}
