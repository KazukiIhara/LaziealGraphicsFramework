#pragma once

namespace LGF {
	/// <summary>
	/// 描画クラスのインターフェース
	/// </summary>
	class IDrawer {
	public:
		virtual ~IDrawer() = default;
		virtual void Initialize() = 0;
		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;
	};
}
