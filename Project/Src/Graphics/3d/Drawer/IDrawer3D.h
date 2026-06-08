#pragma once


namespace LGF {
	/// <summary>
	/// 3D描画クラスのインターフェース
	/// </summary>
	class IDrawer3D {
	public:
		virtual ~IDrawer3D() = default;
		virtual void Initialize() = 0;
		virtual void ThrowDrawCommand() = 0;
	};
}