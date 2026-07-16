#pragma once

namespace LGF {

	class WindowSystem;
	class InputSystem;
	class GraphicsSystem;

	struct FrameworkContext {
		WindowSystem* window = nullptr;
		InputSystem* input = nullptr;
		GraphicsSystem* graphics = nullptr;
	};
}
