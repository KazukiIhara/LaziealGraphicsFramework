#include "LGF/Graphics/Graphics2D.h"

#include <cassert>

#include "Graphics2DInternal.h"
#include "Core/Window/System/WindowSystem.h"
#include "Graphics/Camera/System/Camera2DSystem.h"
#include "Graphics/Renderer/Renderer2D.h"

namespace {
	LGF::WindowSystem* g_windowSystem = nullptr;
	LGF::Camera2DSystem* g_camera2DSystem = nullptr;
	LGF::Renderer2D* g_renderer2D = nullptr;
}

void LGF::Graphics2D::Internal::SetWindowSystem(WindowSystem* windowSystem) {
	assert(windowSystem != nullptr);
	g_windowSystem = windowSystem;
}

void LGF::Graphics2D::Internal::ResetWindowSystem() {
	g_windowSystem = nullptr;
}

void LGF::Graphics2D::Internal::SetCamera2DSystem(Camera2DSystem* cameraSystem) {
	assert(cameraSystem != nullptr);
	g_camera2DSystem = cameraSystem;
}

void LGF::Graphics2D::Internal::ResetCamera2DSystem() {
	g_camera2DSystem = nullptr;
}

void LGF::Graphics2D::Internal::SetRenderer2D(Renderer2D* renderer) {
	assert(renderer != nullptr);
	g_renderer2D = renderer;
}

void LGF::Graphics2D::Internal::ResetRenderer2D() {
	g_renderer2D = nullptr;
}

uint32_t LGF::Graphics2D::GetWindowWidth() {
	return g_windowSystem->GetWindow().GetWndSetting().wndSize.width;
}

uint32_t LGF::Graphics2D::GetWindowHeight() {
	return g_windowSystem->GetWindow().GetWndSetting().wndSize.height;
}

void LGF::Graphics2D::SetCamera(const Camera2D& camera) {
	assert(g_camera2DSystem != nullptr);
	g_camera2DSystem->SetCamera(camera);
}

void LGF::Graphics2D::Draw(const Triangle2D& triangle, const Vector4& color) {
	assert(g_renderer2D != nullptr);
	g_renderer2D->AddTriangle(
		triangle,
		color
	);
}

void LGF::Graphics2D::Draw(const Rect2D& rect, const Vector4& color) {
	assert(g_renderer2D != nullptr);
	g_renderer2D->AddRect(
		rect,
		color
	);
}
