#pragma once

#include "Framework/System/IFrameworkSystem.h"

#include "Window/Window.h"

namespace LGF {
	/// <summary>
	/// ウィンドウ管理クラス
	/// </summary>
	class WindowSystem :public IFrameworkSystem {
	public:
		WindowSystem();
		~WindowSystem()override;

		bool Initialize(FrameworkContext& context)override;
		bool Finalize(FrameworkContext& context)override;

		void ProcessMessage();

		Window& GetWindow();

		bool IsCloseRequested() const;

	private:
		Window window_;
		bool isCloseRequested_ = false;
	};
}