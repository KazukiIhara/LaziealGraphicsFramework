#pragma once

#include <memory>
#include <vector>

namespace LGF {

	class IDrawer3D;

	/// <summary>
	/// 全3D描画をするクラス
	/// </summary>
	class Renderer3D {
	public:
		Renderer3D();
		~Renderer3D() = default;

		void Initialize();

		void BeginFrame();

		void EndFrame();

	private:
		std::vector<std::unique_ptr<IDrawer3D>> drawers_;
	};

}