#pragma once

namespace LGF {

	/// <summary>
	/// 3D描画クラスのインターフェース
	/// </summary>
	class IDrawer3D {
	public:
		virtual ~IDrawer3D() = default;

		virtual void Initialize() = 0;
		virtual void Update() = 0;
		virtual void Draw() = 0;

	private:

	};

}