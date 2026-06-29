#include "GraphicsSystem.h"

#include "Core/Framework/FrameworkContext.h"
#include "Core/Window/System/WindowSystem.h"

#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/Pipeline/Manager/PipelineManager.h"
#include "Graphics/Camera/System/Camera3DSystem.h"
#include "Graphics/Renderer/Renderer3D.h"
#include "Graphics/DirectX/ViewManagers/SRVUAVManager/SRVUAVManager.h"
#include "Graphics/ImGui/ImGuiSystem.h"

#include "Graphics/Graphics3DContext.h"
#include "Graphics/Graphics3DInternal.h"

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

	// 3Dカメラシステム初期化
	camera3DSystem_ = std::make_unique<Camera3DSystem>(*directX_);
	camera3DSystem_->Initialize();
	Graphics3D::Internal::SetCamera3DSystem(camera3DSystem_.get());
	// サブシステムリストに追加
	subsystems_.push_back(camera3DSystem_.get());

	// 3Dレンダラークラス初期化
	Graphics3DContext graphics3DContext{
		*directX_,
		*pipelineManager_,
		*camera3DSystem_
	};
	renderer3D_ = std::make_unique<Renderer3D>(graphics3DContext);
	if (!renderer3D_->Initialize()) {
		return false;
	}
	Graphics3D::Internal::SetRenderer3D(renderer3D_.get());
	// サブシステムリストに追加
	subsystems_.push_back(renderer3D_.get());


	// ImGuiシステムクラス初期化
	imguiSystem_ = std::make_unique<ImGuiSystem>(*directX_, context.window->GetWindow());
	imguiSystem_->Initialize();
	// サブシステムリストに追加
	subsystems_.push_back(imguiSystem_.get());

	return true;
}

bool GraphicsSystem::Finalize([[maybe_unused]] FrameworkContext& context) {
	Graphics3D::Internal::ResetRenderer3D();
	Graphics3D::Internal::ResetCamera3DSystem();

	for (auto& subSys : subsystems_) {
		subSys->Finalize();
	}
	pipelineManager_->Finalize();
	directX_->Finalize();

	renderer3D_.reset();
	camera3DSystem_.reset();
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
	// ディスクリプターヒープセット
	ID3D12GraphicsCommandList* commandList = directX_->GetCommandList();

	// SRV用ヒープを設定
	ID3D12DescriptorHeap* heaps[] = {
		directX_->GetResMgr().srvuav->GetDescriptorHeap()
	};
	commandList->SetDescriptorHeaps(_countof(heaps), heaps);

	for (auto& subSys : subsystems_) {
		subSys->EndFrame();
	}
	directX_->EndFrame();
}
