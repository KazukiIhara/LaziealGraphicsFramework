#pragma once

namespace LGF {

	class WindowSystem;
	class GraphicsSystem;

	struct FrameworkContext {
		WindowSystem* window = nullptr;
		GraphicsSystem* graphics = nullptr;
	};
}