#pragma once

// C++
#include <memory>
#include <vector>

// Framework
#include "Framework/System/IFrameworkSystem.h"
#include "Framework/FrameworkContext.h"

namespace LGF {
	/// <summary>
	/// フレームワーク
	/// </summary>
	class Framework {
	public:
		Framework();
		~Framework() = default;

		bool Initialize();
		bool Update();
		bool Finalize();

	private:
		// フレーム開始処理
		void BeginCurrentFrame();
		// フレーム終了処理
		void CompletePreviousFrame();

	private:
		// フレームワークシステム
		std::vector<std::unique_ptr<IFrameworkSystem>> systems_;
		// フレームワークシステムの参照用構造体
		FrameworkContext context_;
		bool isFrameStarted_ = false;
	};
}

