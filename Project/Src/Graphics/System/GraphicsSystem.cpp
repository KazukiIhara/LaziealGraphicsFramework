#include "GraphicsSystem.h"

#include <filesystem>

#include "Core/Logger/Logger.h"
#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/Pipeline/Manager/PipelineManager.h"
#include "Graphics/Camera/System/Camera3DSystem.h"
#include "Graphics/Light/System/DirectionalLightSystem.h"
#include "Graphics/Light/System/PointLightSystem.h"
#include "Graphics/Skybox/System/SkyboxSystem.h"
#include "Graphics/Shadow/System/ShadowMapSystem.h"
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

GraphicsSystem::~GraphicsSystem() {
	if (directX_ || isLoggerInitialized_) {
		Finalize();
	}
}

bool GraphicsSystem::Initialize(const GraphicsInitDesc& desc) {
	if (desc.windowHandle == nullptr || desc.width == 0 || desc.height == 0 ||
		desc.assetRoot.empty()) {
		return false;
	}
	if (!std::filesystem::is_directory(desc.assetRoot / "Shaders") ||
		!std::filesystem::is_directory(desc.assetRoot / "JsonData" / "Pipeline")) {
		return false;
	}

	Logger::Initialize();
	isLoggerInitialized_ = true;
	const auto failInitialization = [this]() {
		Finalize();
		return false;
	};

	// DirectX初期化
	directX_ = std::make_unique<DirectXCore>(
		desc.windowHandle,
		desc.width,
		desc.height,
		desc.multiFrameContextEnabled,
		desc.vsyncEnabled);
	if (!directX_->Initialize()) {
		return failInitialization();
	}

	// テクスチャマネージャ初期化
	textureManager_ = std::make_unique<TextureManager>(*directX_);
	if (!textureManager_->Initialize()) {
		return failInitialization();
	}
	TextureSystem::Internal::SetTextureManager(textureManager_.get());

	// 静的モデルマネージャ初期化
	modelManager_ = std::make_unique<ModelManager>(*directX_, *textureManager_);
	if (!modelManager_->Initialize()) {
		return failInitialization();
	}
	ModelSystem::Internal::SetModelManager(modelManager_.get());

	// パイプラインマネージャ初期化
	pipelineManager_ = std::make_unique<PipelineManager>(*directX_, desc.assetRoot);
	pipelineManager_->Initialize();

	// 3Dカメラシステム初期化
	Graphics3D::Internal::SetDirectXCore(directX_.get());
	camera3DSystem_ = std::make_unique<Camera3DSystem>(*directX_);
	camera3DSystem_->Initialize();
	Graphics3D::Internal::SetCamera3DSystem(camera3DSystem_.get());
	// サブシステムリストに追加
	subsystems_.push_back(camera3DSystem_.get());

	// 平行光源システム初期化
	directionalLightSystem_ = std::make_unique<DirectionalLightSystem>(*directX_);
	if (!directionalLightSystem_->Initialize()) {
		return failInitialization();
	}
	Graphics3D::Internal::SetDirectionalLightSystem(directionalLightSystem_.get());
	subsystems_.push_back(directionalLightSystem_.get());

	// 点光源システム初期化
	pointLightSystem_ = std::make_unique<PointLightSystem>(*directX_);
	if (!pointLightSystem_->Initialize()) {
		return failInitialization();
	}
	Graphics3D::Internal::SetPointLightSystem(pointLightSystem_.get());
	subsystems_.push_back(pointLightSystem_.get());

	shadowMapSystem_ = std::make_unique<ShadowMapSystem>(*directX_);
	if (!shadowMapSystem_->Initialize()) {
		return failInitialization();
	}

	// Skyboxシステム初期化
	skyboxSystem_ = std::make_unique<SkyboxSystem>(*directX_, *pipelineManager_);
	if (!skyboxSystem_->Initialize()) {
		return failInitialization();
	}
	Graphics3D::Internal::SetSkyboxSystem(skyboxSystem_.get());
	subsystems_.push_back(skyboxSystem_.get());

	// 3Dレンダラークラス初期化
	Graphics3DContext graphics3DContext{
		*directX_,
		*pipelineManager_,
		*camera3DSystem_
	};
	renderer3D_ = std::make_unique<Renderer3D>(graphics3DContext);
	if (!renderer3D_->Initialize()) {
		return failInitialization();
	}
	Graphics3D::Internal::SetRenderer3D(renderer3D_.get());
	// サブシステムリストに追加
	subsystems_.push_back(renderer3D_.get());

	// Windowシステムをセット
	Graphics2D::Internal::SetDirectXCore(directX_.get());
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
		return failInitialization();
	}
	Graphics2D::Internal::SetRenderer2D(renderer2D_.get());
	// サブシステムリストに追加
	subsystems_.push_back(renderer2D_.get());

	// ImGuiシステムクラス初期化
	imguiSystem_ = std::make_unique<ImGuiSystem>(*directX_, desc.windowHandle);
	imguiSystem_->Initialize();
	// サブシステムリストに追加
	subsystems_.push_back(imguiSystem_.get());

	return true;
}

bool GraphicsSystem::Finalize() {
	if (directX_) {
		directX_->WaitIdle();
	}
	ModelSystem::Internal::ResetModelManager();
	TextureSystem::Internal::ResetTextureManager();

	Graphics3D::Internal::ResetRenderer3D();
	Graphics3D::Internal::ResetCamera3DSystem();
	Graphics3D::Internal::ResetDirectionalLightSystem();
	Graphics3D::Internal::ResetPointLightSystem();
	Graphics3D::Internal::ResetSkyboxSystem();
	Graphics3D::Internal::ResetDirectXCore();

	Graphics2D::Internal::ResetDirectXCore();
	Graphics2D::Internal::ResetRenderer2D();
	Graphics2D::Internal::ResetCamera2DSystem();

	for (auto& subSys : subsystems_) {
		subSys->Finalize();
	}
	subsystems_.clear();
	if (pipelineManager_) {
		pipelineManager_->Finalize();
	}
	if (modelManager_) {
		modelManager_->Finalize();
	}
	if (textureManager_) {
		textureManager_->Finalize();
	}
	if (directX_) {
		directX_->Finalize();
	}

	renderer2D_.reset();
	camera2DSystem_.reset();
	renderer3D_.reset();
	shadowMapSystem_.reset();
	directionalLightSystem_.reset();
	pointLightSystem_.reset();
	skyboxSystem_.reset();
	camera3DSystem_.reset();
	pipelineManager_.reset();
	modelManager_.reset();
	textureManager_.reset();
	directX_.reset();
	isFrameRenderable_ = false;
	if (isLoggerInitialized_) {
		Logger::Finalize();
		isLoggerInitialized_ = false;
	}
	return true;
}

bool GraphicsSystem::Resize(uint32_t width, uint32_t height) {
	return directX_ && directX_->Resize(width, height);
}

void GraphicsSystem::BeginFrame() {
	isFrameRenderable_ = directX_->BeginFrame();
	if (!isFrameRenderable_) {
		return;
	}
	for (auto& subSys : subsystems_) {
		subSys->BeginFrame();
	}
}

void GraphicsSystem::EndFrame() {
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
	pointLightSystem_->EndFrame();
	shadowMapSystem_->Update(
		camera3DSystem_->GetShadowCamera(),
		directionalLightSystem_->GetLight());
	for (uint32_t cascadeIndex = 0;
		cascadeIndex < ShadowCascadeCalculator::kCascadeCount;
		++cascadeIndex) {
		shadowMapSystem_->BeginShadowPass(cascadeIndex);
		renderer3D_->DrawShadow(*shadowMapSystem_);
		shadowMapSystem_->EndShadowPass();
	}
	renderer3D_->EndFrame();
	skyboxSystem_->EndFrame();

	directX_->DrawLightingToSceneColor(
		*pipelineManager_,
		*directionalLightSystem_,
		*pointLightSystem_,
		*camera3DSystem_,
		*skyboxSystem_,
		*shadowMapSystem_);
	directX_->DrawSceneToSwapChain(*pipelineManager_);

	// 2DとImGuiはトーンマッピング後のSwapChainへ描画する。
	camera2DSystem_->EndFrame();
	renderer2D_->EndFrame();
	imguiSystem_->EndFrame();

	directX_->EndFrame();
	isFrameRenderable_ = false;
}

bool GraphicsSystem::HandleWindowMessage(
	void* windowHandle,
	uint32_t message,
	uintptr_t wParam,
	intptr_t lParam) {
	return imguiSystem_ && imguiSystem_->HandleWindowMessage(
		windowHandle,
		message,
		wParam,
		lParam);
}
