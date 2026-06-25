#include "Framework.h"

#include <ranges>

#include "Core/Window/System/WindowSystem.h"
#include "Graphics/System/GraphicsSystem.h"

using namespace LGF;

Framework::Framework() {
}

Framework::~Framework() = default;

bool Framework::Initialize() {
	// ウィンドウシステム
	std::unique_ptr<WindowSystem> windowSys = std::make_unique<WindowSystem>();
	context_.window = windowSys.get();
	systems_.push_back(std::move(windowSys));

	// グラフィックシステム
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
	// 前フレームに積んだコマンドの実行
	if (isFrameStarted_) {
		EndFrame();
		isFrameStarted_ = false;
	}

	// OSメッセージ処理
	context_.window->ProcessMessage();

	// 終了要求を確認
	if (context_.window->IsCloseRequested()) {
		return false;
	}

	// フレーム開始処理
	BeginFrame();

	isFrameStarted_ = true;

	return true;
}

bool Framework::Finalize() {
	if (isFrameStarted_) {
		EndFrame();
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

void Framework::BeginFrame() {
	for (const auto& system : std::views::reverse(systems_)) {
		system->BeginFrame(context_);
	}
}

void Framework::EndFrame() {
	for (const auto& system : std::views::reverse(systems_)) {
		system->EndFrame(context_);
	}
}
