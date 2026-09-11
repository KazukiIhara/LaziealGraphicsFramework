#pragma once

#include <chrono>

#include "Core/Framework/System/IFrameworkSystem.h"

namespace LGF {

	/// <summary>
	/// フレーム間の経過時間を管理するシステム
	/// </summary>
	class TimeSystem final : public IFrameworkSystem {
	public:
		TimeSystem();
		~TimeSystem() override;

		bool Initialize(FrameworkContext& context) override;
		bool Finalize(FrameworkContext& context) override;

		void BeginFrame(FrameworkContext& context) override;

		double GetDeltaTime() const;

	private:
		using Clock = std::chrono::steady_clock;

		Clock::time_point previousFrameTime_{};
		double deltaTime_ = 0.0;
		bool isFirstFrame_ = true;
	};

}
