#pragma once

namespace LGF {

	class WindowSystem;

	namespace Graphics2D::Internal {

		void SetWindowSystem(WindowSystem* windowSystem);
		void ResetWindowSystem();

	}

}