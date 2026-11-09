#include "ParticleDrawer3D.h"

#include <stdexcept>

#include "Graphics/Camera/System/Camera3DSystem.h"
#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/DirectX/ViewManagers/SRVUAVManager/SRVUAVManager.h"
#include "Graphics/Pipeline/Manager/PipelineManager.h"
#include "Graphics/Pipeline/Pipeline.h"

using namespace LGF;

ParticleDrawer3D::ParticleDrawer3D(Graphics3DContext context) : BaseDrawer3D(context) {
}

void ParticleDrawer3D::Initialize() {
	BaseDrawer::Initialize({ kPageCapacity, sizeof(ParticleBillboardForGPU), reinterpret_cast<void**>(&instanceData_) });
	SetPipeline(GetContext().pipelineMgr.GetPipeline("ParticleBillboard3D"));
	if (!GetPipeline() || !GetPipeline()->IsValid()) {
		throw std::runtime_error("ParticleBillboard3D pipeline is unavailable.");
	}
}

void ParticleDrawer3D::BeginFrame() {
	BaseDrawer::BeginFrame();
	currentIndex_ = 0u;
	if (nextPage_) {
		nextPage_->BeginFrame();
	}
}

void ParticleDrawer3D::EndFrame() {
	if (CheckIntstanceCount() != 0u) {
		auto* commandList = GetContext().directX.GetCommandList6();
		commandList->SetGraphicsRootSignature(GetPipeline()->GetRootSignature());
		commandList->SetPipelineState(GetPipeline()->GetPipelineState());
		GetContext().cameraSys.Bind(commandList, GetPipeline()->GetRootIndex("gCamera"));
		commandList->SetGraphicsRootDescriptorTable(GetPipeline()->GetRootIndex("gParticle"),
			GetContext().directX.GetResMgr().srvuav->GetGPUDescriptorHandle(instanceSrvIndex_));
		commandList->DispatchMesh(instanceCount_, 1, 1);
	}
	if (nextPage_) {
		nextPage_->EndFrame();
	}
}

void ParticleDrawer3D::AddParticle(const ParticleBillboardForGPU& data) {
	if (currentIndex_ < numMaxInstance_) {
		instanceData_[currentIndex_++] = data;
		return;
	}
	if (!nextPage_) {
		nextPage_ = std::make_unique<ParticleDrawer3D>(GetContext());
		nextPage_->Initialize();
	}
	// 別ページのバッファへ書き込むため、先に積んだ描画データを上書きしない。
	// フレームごとのバッファ切替は既存のBaseDrawerに任せる。
	nextPage_->AddParticle(data);
}
