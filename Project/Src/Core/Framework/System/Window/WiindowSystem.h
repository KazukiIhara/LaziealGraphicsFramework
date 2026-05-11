#pragma once

#include "Framework/System/IFrameworkSystem.h"

namespace LGF {
	/// <summary>
	/// ウィンドウ管理クラス
	/// </summary>
	class WindowSystem :public IFrameworkSystem {
	public:
		WindowSystem();
		~WindowSystem()override = default;

		bool Initialize();

	private:

	};
}