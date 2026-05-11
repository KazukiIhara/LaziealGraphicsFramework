#include "WindowSystem.h"

using namespace LGF;

WindowSystem::WindowSystem() {
}

bool WindowSystem::Initialize([[maybe_unused]] FrameworkContext& context) {
	return window_.Initialize();
}

bool WindowSystem::Update([[maybe_unused]] FrameworkContext& context) {
	return window_.ProcessMessage();
}

bool WindowSystem::Finalize([[maybe_unused]] FrameworkContext& context) {
	return window_.Finalize();
}

Window& WindowSystem::GetWindow() {
	return window_;
}
