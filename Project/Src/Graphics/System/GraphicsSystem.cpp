#include "GraphicsSystem.h"

#include "Framework/FrameworkContext.h"
#include "Window/System/WindowSystem.h"

using namespace LGF;

GraphicsSystem::GraphicsSystem() {
}

bool GraphicsSystem::Initialize(FrameworkContext& context) {
	directX_ = std::make_unique<DirectXCore>();
	return directX_->Initialize(context.window->GetWindow());
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
	directX_->EndFrame();
}
