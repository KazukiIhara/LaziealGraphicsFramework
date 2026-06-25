#include "LGF/Graphics/Graphics2D.h"

#include <cassert>

#include "Graphics2DInternal.h"
#include "Core/Window/System/WindowSystem.h"

namespace {
	LGF::WindowSystem* g_windowSystem = nullptr;
}

void LGF::Graphics2D::Internal::SetWindowSystem(WindowSystem* windowSystem) {
	assert(windowSystem != nullptr);
	g_windowSystem = windowSystem;
}

void LGF::Graphics2D::Internal::ResetWindowSystem() {
	g_windowSystem = nullptr;
}


uint32_t LGF::Graphics2D::GetWindowWidth() {
	return g_windowSystem->GetWindow().GetWndSetting().wndSize.width;
}

uint32_t LGF::Graphics2D::GetWindowHeight() {
	return g_windowSystem->GetWindow().GetWndSetting().wndSize.height;
}