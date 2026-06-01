#pragma once

#include <cstdint>

namespace LGF {

	/// <summary>
	/// プリミティブ描画クラス
	/// </summary>
	class PrimitiveDrawer {
	public:
		PrimitiveDrawer();
		~PrimitiveDrawer() = default;

		void AddPrimitive();

		void Initialize();

		void Update();

		void Draw();

	private:
		// instancing描画のリソース

		// instancing描画用のデータ


		uint32_t

	};

}

