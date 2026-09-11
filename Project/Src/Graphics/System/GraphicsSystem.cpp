#include "GraphicsSystem.h"

#include "Core/Framework/FrameworkContext.h"
#include "Core/Window/System/WindowSystem.h"

#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/Pipeline/Manager/PipelineManager.h"
#include "Graphics/Camera/System/Camera3DSystem.h"
#include "Graphics/Light/System/DirectionalLightSystem.h"
#include "Graphics/Renderer/Renderer3D.h"
#include "Graphics/Camera/System/Camera2DSystem.h"
#include "Graphics/Renderer/Renderer2D.h"
#include "Graphics/DirectX/ViewManagers/SRVUAVManager/SRVUAVManager.h"
#include "Graphics/ImGui/ImGuiSystem.h"
#include "Graphics/Texture/TextureManager.h"
#include "Graphics/Texture/TextureSystemInternal.h"
#include "Graphics/Model/ModelManager.h"
#include "Graphics/Model/ModelSystemInternal.h"

#include "Graphics/Graphics3DContext.h"
#include "Graphics/Graphics3DInternal.h"
#include "Graphics/Graphics2DContext.h"
#include "Graphics/Graphics2DInternal.h"

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

	// テクスチャマネージャ初期化
	textureManager_ = std::make_unique<TextureManager>(*directX_);
	if (!textureManager_->Initialize()) {
		return false;
	}
	TextureSystem::Internal::SetTextureManager(textureManager_.get());

	// 静的モデルマネージャ初期化
	modelManager_ = std::make_unique<ModelManager>(*directX_, *textureManager_);
	if (!modelManager_->Initialize()) {
		return false;
	}
	ModelSystem::Internal::SetModelManager(modelManager_.get());

	// パイプラインマネージャ初期化
	pipelineManager_ = std::make_unique<PipelineManager>(*directX_);
	pipelineManager_->Initialize();

	// 3Dカメラシステム初期化
	Graphics3D::Internal::SetWindowSystem(context.window);
	camera3DSystem_ = std::make_unique<Camera3DSystem>(*directX_);
	camera3DSystem_->Initialize();
	Graphics3D::Internal::SetCamera3DSystem(camera3DSystem_.get());
	// サブシステムリストに追加
	subsystems_.push_back(camera3DSystem_.get());

	// 平行光源システム初期化
	directionalLightSystem_ = std::make_unique<DirectionalLightSystem>(*directX_);
	if (!directionalLightSystem_->Initialize()) {
		return false;
	}
	Graphics3D::Internal::SetDirectionalLightSystem(directionalLightSystem_.get());
	subsystems_.push_back(directionalLightSystem_.get());

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

	// Windowシステムをセット
	Graphics2D::Internal::SetWindowSystem(context.window);
	// 2Dカメラシステム初期化
	camera2DSystem_ = std::make_unique<Camera2DSystem>(*directX_);
	camera2DSystem_->Initialize();
	Graphics2D::Internal::SetCamera2DSystem(camera2DSystem_.get());
	// サブシステムリストに追加
	subsystems_.push_back(camera2DSystem_.get());

	// 2Dレンダラークラス初期化
	Graphics2DContext graphics2DContext{
		*directX_,
		*pipelineManager_,
		*camera2DSystem_
	};
	renderer2D_ = std::make_unique<Renderer2D>(graphics2DContext);
	if (!renderer2D_->Initialize()) {
		return false;
	}
	Graphics2D::Internal::SetRenderer2D(renderer2D_.get());
	// サブシステムリストに追加
	subsystems_.push_back(renderer2D_.get());

	// ImGuiシステムクラス初期化
	imguiSystem_ = std::make_unique<ImGuiSystem>(*directX_, context.window->GetWindow());
	imguiSystem_->Initialize();
	// サブシステムリストに追加
	subsystems_.push_back(imguiSystem_.get());

	return true;
}

bool GraphicsSystem::Finalize([[maybe_unused]] FrameworkContext& context) {
	ModelSystem::Internal::ResetModelManager();
	TextureSystem::Internal::ResetTextureManager();

	Graphics3D::Internal::ResetRenderer3D();
	Graphics3D::Internal::ResetCamera3DSystem();
	Graphics3D::Internal::ResetDirectionalLightSystem();
	Graphics3D::Internal::ResetWindowSystem();

	Graphics2D::Internal::ResetWindowSystem();
	Graphics2D::Internal::ResetRenderer2D();
	Graphics2D::Internal::ResetCamera2DSystem();

	for (auto& subSys : subsystems_) {
		subSys->Finalize();
	}
	pipelineManager_->Finalize();
	modelManager_->Finalize();
	textureManager_->Finalize();
	directX_->Finalize();

	renderer2D_.reset();
	camera2DSystem_.reset();
	renderer3D_.reset();
	directionalLightSystem_.reset();
	camera3DSystem_.reset();
	pipelineManager_.reset();
	modelManager_.reset();
	textureManager_.reset();
	directX_.reset();
	return true;
}

void GraphicsSystem::BeginFrame([[maybe_unused]] FrameworkContext& context) {
	isFrameRenderable_ = directX_->BeginFrame();
	if (!isFrameRenderable_) {
		return;
	}
	for (auto& subSys : subsystems_) {
		subSys->BeginFrame();
	}
}

void GraphicsSystem::EndFrame([[maybe_unused]] FrameworkContext& context) {
	if (!isFrameRenderable_) {
		return;
	}

	// ディスクリプターヒープセット
	ID3D12GraphicsCommandList* commandList = directX_->GetCommandList();

	// SRV用ヒープを設定
	ID3D12DescriptorHeap* heaps[] = {
		directX_->GetResMgr().srvuav->GetDescriptorHeap()
	};
	commandList->SetDescriptorHeaps(_countof(heaps), heaps);

	camera3DSystem_->EndFrame();
	directionalLightSystem_->EndFrame();
	renderer3D_->EndFrame();

	directX_->DrawLightingToSceneColor(*pipelineManager_, *directionalLightSystem_);

	camera2DSystem_->EndFrame();
	renderer2D_->EndFrame();
	imguiSystem_->EndFrame();

	directX_->DrawSceneToSwapChain(*pipelineManager_);
	directX_->EndFrame();
	isFrameRenderable_ = false;
}
