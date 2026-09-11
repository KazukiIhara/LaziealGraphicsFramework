#pragma once

// C++
#include <memory>
#include <vector>

// Framework
#include "Core/Framework/System/IFrameworkSystem.h"
#include "Core/Framework/FrameworkContext.h"

namespace LGF {
	/// <summary>
	/// フレームワーク
	/// </summary>
	class Framework {
	public:
		Framework();
		~Framework();

		bool Initialize();
		bool Update();
		bool Finalize();

		double GetDeltaTime() const;

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
		bool isFrameStarted_ = false;
	};
}

