#include "GraphicsSystem.h"

#include "Framework/FrameworkContext.h"
#include "Window/System/WindowSystem.h"

#include "DirectX/Core/DirectXCore.h"
#include "Pipeline/Manager/PipelineManager.h"
#include "3d/Renderer/Renderer3D.h"

using namespace LGF;

GraphicsSystem::GraphicsSystem() {
}

GraphicsSystem::~GraphicsSystem() = default;

bool GraphicsSystem::Initialize(FrameworkContext& context) {
	// DirectX初期化
	directX_ = std::make_unique<DirectXCore>(context.window->GetWindow());
	if (!directX_->Initialize()) {
		return false;
	}

	// パイプラインマネージャ初期化
	pipelineManager_ = std::make_unique<PipelineManager>(*directX_);
	pipelineManager_->Initialize();

	// 3Dレンダラークラス初期化
	renderer3D_ = std::make_unique<Renderer3D>(*pipelineManager_);
	renderer3D_->Initialize();
	// サブシステムリストに追加
	subsystems_.push_back(renderer3D_.get());

	return true;
}

bool GraphicsSystem::Update([[maybe_unused]] FrameworkContext& context) {
	return true;
}

bool GraphicsSystem::Finalize([[maybe_unused]] FrameworkContext& context) {
	for (auto& subSys : subsystems_) {
		subSys->Finalize();
	}
	pipelineManager_->Finalize();
	directX_->Finalize();

	renderer3D_.reset();
	pipelineManager_.reset();
	directX_.reset();
	return true;
}

void GraphicsSystem::BeginFrame([[maybe_unused]] FrameworkContext& context) {
	directX_->BeginFrame();
	for (auto& subSys : subsystems_) {
		subSys->BeginFrame();
	}
}

void GraphicsSystem::EndFrame([[maybe_unused]] FrameworkContext& context) {
	for (auto& subSys : subsystems_) {
		subSys->EndFrame();
	}
	directX_->EndFrame();
}
