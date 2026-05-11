#include "GraphicsSystem.h"

using namespace LGF;

GraphicsSystem::GraphicsSystem() {
}

bool GraphicsSystem::Initialize([[maybe_unused]] FrameworkContext& context) {
	return false;
}

bool GraphicsSystem::Update([[maybe_unused]] FrameworkContext& context) {
	return false;
}

bool GraphicsSystem::Finalize([[maybe_unused]] FrameworkContext& context) {
	return false;
}

void GraphicsSystem::BeginFrame([[maybe_unused]] FrameworkContext& context) {
}

void GraphicsSystem::EndFrame([[maybe_unused]] FrameworkContext& context) {
}
