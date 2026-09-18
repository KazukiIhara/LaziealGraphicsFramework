#include "LGF/Graphics/Graphics2D.h"

#include <cassert>

#include "Graphics2DInternal.h"
#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/Camera/System/Camera2DSystem.h"
#include "Graphics/Renderer/Renderer2D.h"

namespace {
	LGF::DirectXCore* g_directX = nullptr;
	LGF::Camera2DSystem* g_camera2DSystem = nullptr;
	LGF::Renderer2D* g_renderer2D = nullptr;
}

void LGF::Graphics2D::Internal::SetDirectXCore(DirectXCore* directX) {
	assert(directX != nullptr);
	g_directX = directX;
}

void LGF::Graphics2D::Internal::ResetDirectXCore() {
	g_directX = nullptr;
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
	assert(g_directX != nullptr);
	return g_directX->GetRenderWidth();
}

uint32_t LGF::Graphics2D::GetWindowHeight() {
	assert(g_directX != nullptr);
	return g_directX->GetRenderHeight();
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

void LGF::Graphics2D::Draw(const Circle2D& circle, const Vector4& color) {
	assert(g_renderer2D != nullptr);
	g_renderer2D->AddCircle(
		circle,
		color
	);
}

void LGF::Graphics2D::Draw(const Line2D& line, const Vector4& color) {
	assert(g_renderer2D != nullptr);
	g_renderer2D->AddLine(
		line,
		color
	);
}

void LGF::Graphics2D::Draw(const RoundRect2D& roundRect, const Vector4& color) {
	assert(g_renderer2D != nullptr);
	g_renderer2D->AddRoundRect(
		roundRect,
		color
	);
}
