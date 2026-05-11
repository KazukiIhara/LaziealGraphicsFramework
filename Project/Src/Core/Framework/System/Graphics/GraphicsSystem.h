#pragma once

#include "Framework/System/IFrameworkSystem.h"

namespace LGF {
	/// <summary>
	/// グラフィックシステムクラス
	/// </summary>
	class GraphicsSystem :public IFrameworkSystem {
	public:
		GraphicsSystem();
		~GraphicsSystem()override = default;

		bool Initialize(FrameworkContext& context)override;
		bool Update(FrameworkContext& context)override;
		bool Finalize(FrameworkContext& context)override;

		void BeginFrame(FrameworkContext& context)override;
		void EndFrame(FrameworkContext& context)override;

	private:

	};
}
