#pragma once

// C++
#include <memory>
#include <vector>

// Framework
#include "Framework/System/IFrameworkSystem.h"
#include "Framework/Context/FrameworkContext.h"

namespace LGF {
	/// <summary>
	/// フレームワーク
	/// </summary>
	class Framework {
	public:
		Framework();
		~Framework();

		bool Initialize();
		void Finalize();

		bool Update();

	private:
		// フレーム開始処理
		void BeginFrame();
		// フレーム終了処理
		void EndFrame();

	private:
		// フレームワークシステム
		std::vector<std::unique_ptr<IFrameworkSystem>> systems_;
		// フレームワークシステムの参照用構造体
		FrameworkContext context_;
	};
}

