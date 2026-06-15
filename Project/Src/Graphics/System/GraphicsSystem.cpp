#include "GraphicsSystem.h"

#include "Framework/FrameworkContext.h"
#include "Window/System/WindowSystem.h"
#include "DirectX/Core/DirectXCore.h"

#include "3d/Renderer/Renderer3D.h"

using namespace LGF;

GraphicsSystem::GraphicsSystem() {
}

GraphicsSystem::~GraphicsSystem() = default;

bool GraphicsSystem::Initialize(FrameworkContext& context) {
	// DirectX初期化
	directX_ = std::make_unique<DirectXCore>();
	if (!directX_->Initialize(context.window->GetWindow())) {
		return false;
	}

	// 3Dレンダラークラス初期化
	renderer3D_->Initialize();

	return true;
}

bool GraphicsSystem::Update([[maybe_unused]] FrameworkContext& context) {
	return true;
}

bool GraphicsSystem::Finalize([[maybe_unused]] FrameworkContext& context) {
	directX_->Finalize();
	directX_.reset();
	return true;
}

void GraphicsSystem::BeginFrame([[maybe_unused]] FrameworkContext& context) {
	directX_->BeginFrame();
}

void GraphicsSystem::EndFrame([[maybe_unused]] FrameworkContext& context) {
	renderer3D_->EndFrame();
	directX_->EndFrame();
}
