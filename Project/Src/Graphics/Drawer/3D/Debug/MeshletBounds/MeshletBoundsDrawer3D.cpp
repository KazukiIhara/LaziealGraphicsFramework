#include "MeshletBoundsDrawer3D.h"

#include <cassert>
#include <stdexcept>

#include "Graphics/Camera/System/Camera3DSystem.h"
#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/DirectX/ViewManagers/SRVUAVManager/SRVUAVManager.h"
#include "Graphics/Pipeline/Manager/PipelineManager.h"
#include "Graphics/Pipeline/Pipeline.h"

using namespace LGF;

MeshletBoundsDrawer3D::MeshletBoundsDrawer3D(Graphics3DContext context) :
	BaseDrawer3D(context) {
}

MeshletBoundsDrawer3D::~MeshletBoundsDrawer3D() = default;

void MeshletBoundsDrawer3D::Initialize() {
	BaseDrawer::InitParam initParam{
		kMaxBoundsCount,
		sizeof(MeshletBounds3DForGPU),
		reinterpret_cast<void**>(&instanceData_)
	};
	BaseDrawer::Initialize(initParam);

	SetPipeline(GetContext().pipelineMgr.GetPipeline("MeshletBounds3D"));
	assert(GetPipeline() != nullptr);
}

void MeshletBoundsDrawer3D::EndFrame() {
	const RenderPassInstanceRange range = GetCurrentRenderPassRange();
	if (range.count == 0u) {
		return;
	}

	ID3D12GraphicsCommandList6* commandList = GetContext().directX.GetCommandList6();
	commandList->SetGraphicsRootSignature(GetPipeline()->GetRootSignature());
	commandList->SetPipelineState(GetPipeline()->GetPipelineState());

	GetContext().cameraSys.Bind(commandList, GetPipeline()->GetRootIndex("gCamera"));
	BindDrawRange(GetPipeline());
	commandList->SetGraphicsRootDescriptorTable(
		GetPipeline()->GetRootIndex("gMeshletBounds"),
		GetContext().directX.GetResMgr().srvuav->GetGPUDescriptorHandle(instanceSrvIndex_));

	commandList->DispatchMesh(range.count, 1, 1);
}

void MeshletBoundsDrawer3D::AddBounds(const MeshletBounds3DForGPU& data) {
	if (currentIndex_ >= numMaxInstance_) {
		throw std::runtime_error("Too many meshlet bounding spheres were submitted in one frame.");
	}

	AddInstance(&data);
}
