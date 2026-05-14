#include "Framework.h"

#include <ranges>

#include "Window/System/WindowSystem.h"
#include "Graphics/System/GraphicsSystem.h"

using namespace LGF;

Framework::Framework() {
}

bool Framework::Initialize() {
	// ウィンドウシステム
	std::unique_ptr<WindowSystem> windowSys = std::make_unique<WindowSystem>();
	context_.window = windowSys.get();
	systems_.push_back(std::move(windowSys));

	std::unique_ptr<GraphicsSystem> graphicsSys = std::make_unique<GraphicsSystem>();
	context_.graphics = graphicsSys.get();
	systems_.push_back(std::move(graphicsSys));


	// 各システムを初期化
	for (const auto& system : systems_) {
		if (!system->Initialize(context_)) {
			return false;
		}
	}

	return true;
}

bool Framework::Update() {
	if (isFrameStarted_) {
		CompletePreviousFrame();
		isFrameStarted_ = false;
	}
	// 各システムを更新
	for (const auto& system : systems_) {
		if (!system->Update(context_)) {
			return false;
		}
	}

	BeginCurrentFrame();
	isFrameStarted_ = true;

	return true;
}

bool Framework::Finalize() {
	if (isFrameStarted_) {
		CompletePreviousFrame();
		isFrameStarted_ = false;
	}

	// 各システムを逆順に終了
	for (const auto& system : std::views::reverse(systems_)) {
		if (!system->Finalize(context_)) {
			return false;
		}
	}

	return true;
}

void Framework::BeginCurrentFrame() {
	for (const auto& system : std::views::reverse(systems_)) {
		system->BeginFrame(context_);
	}
}

void Framework::CompletePreviousFrame() {
	for (const auto& system : std::views::reverse(systems_)) {
		system->EndFrame(context_);
	}
}
