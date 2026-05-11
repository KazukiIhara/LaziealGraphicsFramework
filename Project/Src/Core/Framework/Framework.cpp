#include "Framework.h"

#include <ranges>

#include "Framework/System/Window/WindowSystem.h"
#include "Framework/System/Graphics/GraphicsSystem.h"

using namespace LGF;

Framework::Framework() {
}

bool Framework::Initialize() {
	// ウィンドウシステム
	std::unique_ptr<WindowSystem> windowSys = std::make_unique<WindowSystem>();
	context_.window = windowSys.get();
	systems_.push_back(std::move(windowSys));


	// 各システムを初期化
	for (const auto& system : systems_) {
		if (!system->Initialize(context_)) {
			return false;
		}
	}

	return true;
}

bool Framework::Update() {
	// 各システムを更新
	for (const auto& system : systems_) {
		if (!system->Update(context_)) {
			return false;
		}
	}

	return true;
}

bool Framework::Finalize() {
	// 各システムを逆順に終了
	for (const auto& system : std::views::reverse(systems_)) {
		if (!system->Finalize(context_)) {
			return false;
		}
	}

	return true;
}

void Framework::BeginFrame() {
}

void Framework::EndFrame() {
}
