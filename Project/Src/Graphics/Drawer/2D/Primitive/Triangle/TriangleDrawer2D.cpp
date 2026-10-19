#include "TriangleDrawer2D.h"

#include <cassert>

#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/DirectX/ViewManagers/SRVUAVManager/SRVUAVManager.h"
#include "Graphics/Pipeline/Pipeline.h"
#include "Graphics/Pipeline/Manager/PipelineManager.h"

using namespace LGF;

TriangleDrawer2D::TriangleDrawer2D(Graphics2DContext context) :
	BaseDrawer2D(context) {
}

TriangleDrawer2D::~TriangleDrawer2D() = default;

void TriangleDrawer2D::Initialize() {
	BaseDrawer::InitParam initParam{
		4096,
		sizeof(Triangle2DForGPU),
		reinterpret_cast<void**>(&instanceData_)
	};
	BaseDrawer::Initialize(initParam);

	SetPipeline(GetContext().pipelineMgr.GetPipeline("Triangle2D"));
	assert(GetPipeline() != nullptr);
}

void TriangleDrawer2D::EndFrame() {
	if (CheckIntstanceCount() == 0) {
		return;
	}

	ID3D12GraphicsCommandList6* commandList = GetContext().directX.GetCommandList6();
	commandList->SetGraphicsRootSignature(GetPipeline()->GetRootSignature());
	commandList->SetPipelineState(GetPipeline()->GetPipelineState());
	commandList->SetGraphicsRootDescriptorTable(
		GetPipeline()->GetRootIndex("gTriangle"),
		GetContext().directX.GetResMgr().srvuav->GetGPUDescriptorHandle(instanceSrvIndex_));
	commandList->DispatchMesh(instanceCount_, 1, 1);
}

void TriangleDrawer2D::AddTriangle(const Triangle2DForGPU& data) {
	assert(currentIndex_ < numMaxInstance_);
	instanceData_[currentIndex_] = data;
	++currentIndex_;
}
