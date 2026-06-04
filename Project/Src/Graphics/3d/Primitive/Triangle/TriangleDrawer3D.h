#pragma once

#include "3d/IDrawer3D.h"

namespace LGF {

	/// <summary>
	/// 3D三角形描画クラス
	/// </summary>
	class TriangleDrawer3D :public IDrawer3D {
	public:
		TriangleDrawer3D();
		~TriangleDrawer3D()override = default;

		void Initialize()override;

		void Update()override;

		void Draw()override;

	private:

	};

}