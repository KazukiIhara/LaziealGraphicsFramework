#include "WindowSystem.h"

using namespace LGF;

WindowSystem::WindowSystem() {
}

WindowSystem::~WindowSystem() = default;

bool WindowSystem::Initialize([[maybe_unused]] FrameworkContext& context) {
	return window_.Initialize();
}

bool WindowSystem::Finalize([[maybe_unused]] FrameworkContext& context) {
	return window_.Finalize();
}

void WindowSystem::ProcessMessage() {
	isCloseRequested_ = !window_.ProcessMessage();
}

Window& WindowSystem::GetWindow() {
	return window_;
}

bool WindowSystem::IsCloseRequested() const {
	return isCloseRequested_;
}
