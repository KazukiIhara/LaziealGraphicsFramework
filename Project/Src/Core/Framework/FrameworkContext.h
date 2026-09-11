#pragma once

namespace LGF {

	class WindowSystem;
	class InputSystem;
	class GraphicsSystem;
	class TimeSystem;

	struct FrameworkContext {
		WindowSystem* window = nullptr;
		InputSystem* input = nullptr;
		GraphicsSystem* graphics = nullptr;
		TimeSystem* time = nullptr;
	};
}
