#include "GraphicsSystem.h"

#include "Framework/FrameworkContext.h"
#include "Window/System/WindowSystem.h"

using namespace LGF;

GraphicsSystem::GraphicsSystem() {
}

bool GraphicsSystem::Initialize(FrameworkContext& context) {
	directX_ = std::make_unique<DirectXCore>();
	if (!directX_->Initialize(context.window->GetWindow())) {
		return false;
	}

	triangleRenderer_ = std::make_unique<TriangleRenderer>();
	if (!triangleRenderer_->Initialize(
		directX_->GetDevice(),
		directX_->GetShaderCompiler(),
		"Assets/Pipelines/Triangle.pipeline.json")) {
		return false;
	}

	return true;
}

bool GraphicsSystem::Update([[maybe_unused]] FrameworkContext& context) {
	return true;
}

bool GraphicsSystem::Finalize([[maybe_unused]] FrameworkContext& context) {
	triangleRenderer_.reset();
	directX_->Finalize();
	directX_.reset();
	return true;
}

void GraphicsSystem::BeginFrame([[maybe_unused]] FrameworkContext& context) {
	directX_->BeginFrame();
}

void GraphicsSystem::EndFrame([[maybe_unused]] FrameworkContext& context) {
	triangleRenderer_->Draw(directX_->GetCommandList());
	directX_->EndFrame();
}
