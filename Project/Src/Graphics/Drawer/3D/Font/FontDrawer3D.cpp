#include "FontDrawer3D.h"

#include <stdexcept>

#include "Graphics/Camera/System/Camera3DSystem.h"
#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/DirectX/ViewManagers/SRVUAVManager/SRVUAVManager.h"
#include "Graphics/Pipeline/Manager/PipelineManager.h"
#include "Graphics/Pipeline/Pipeline.h"

using namespace LGF;

FontDrawer3D::FontDrawer3D(Graphics3DContext context) :
	BaseDrawer3D(context) {
}

void FontDrawer3D::Initialize() {
	BaseDrawer::Initialize({
		kPageCapacity,
		sizeof(FontGlyph3DForGPU),
		reinterpret_cast<void**>(&instanceData_),
	});
	SetPipeline(GetContext().pipelineMgr.GetPipeline("Font3D"));
	if (!GetPipeline() || !GetPipeline()->IsValid()) {
		throw std::runtime_error("Font3D pipeline is unavailable.");
	}
}

void FontDrawer3D::BeginFrame() {
	BaseDrawer::BeginFrame();
	currentIndex_ = 0u;
	if (nextPage_) {
		nextPage_->BeginFrame();
	}
}

void FontDrawer3D::EndFrame() {
	if (CheckIntstanceCount() != 0u) {
		auto* commandList = GetContext().directX.GetCommandList6();
		commandList->SetGraphicsRootSignature(GetPipeline()->GetRootSignature());
		commandList->SetPipelineState(GetPipeline()->GetPipelineState());
		GetContext().cameraSys.Bind(
			commandList,
			GetPipeline()->GetRootIndex("gCamera"));
		commandList->SetGraphicsRootDescriptorTable(
			GetPipeline()->GetRootIndex("gGlyph"),
			GetContext().directX.GetResMgr().srvuav->GetGPUDescriptorHandle(instanceSrvIndex_));
		commandList->DispatchMesh(instanceCount_, 1, 1);
	}
	if (nextPage_) {
		nextPage_->EndFrame();
	}
}

void FontDrawer3D::AddGlyph(const FontGlyph3DForGPU& data) {
	if (currentIndex_ < numMaxInstance_) {
		instanceData_[currentIndex_++] = data;
		return;
	}
	if (!nextPage_) {
		nextPage_ = std::make_unique<FontDrawer3D>(GetContext());
		nextPage_->Initialize();
	}
	nextPage_->AddGlyph(data);
}
