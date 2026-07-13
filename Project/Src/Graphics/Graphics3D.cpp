#include "LGF/Graphics/Graphics3D.h"

#include <cassert>

#include "Graphics3DInternal.h"
#include "Core/Window/System/WindowSystem.h"
#include "Graphics/Camera/System/Camera3DSystem.h"
#include "Graphics/Renderer/Renderer3D.h"

namespace {
	LGF::WindowSystem* g_windowSystem = nullptr;
	LGF::Camera3DSystem* g_camera3DSystem = nullptr;
	LGF::Renderer3D* g_renderer3D = nullptr;
}

void LGF::Graphics3D::Internal::SetWindowSystem(WindowSystem* windowSystem) {
	assert(windowSystem != nullptr);
	g_windowSystem = windowSystem;
}

void LGF::Graphics3D::Internal::ResetWindowSystem() {
	g_windowSystem = nullptr;
}

void LGF::Graphics3D::Internal::SetCamera3DSystem(Camera3DSystem* cameraSystem) {
	assert(cameraSystem != nullptr);
	g_camera3DSystem = cameraSystem;
}

void LGF::Graphics3D::Internal::ResetCamera3DSystem() {
	g_camera3DSystem = nullptr;
}

void LGF::Graphics3D::Internal::SetRenderer3D(Renderer3D* renderer) {
	assert(renderer != nullptr);
	g_renderer3D = renderer;
}

void LGF::Graphics3D::Internal::ResetRenderer3D() {
	g_renderer3D = nullptr;
}

uint32_t LGF::Graphics3D::GetWindowWidth() {
	return g_windowSystem->GetWindow().GetWndSetting().wndSize.width;
}

uint32_t LGF::Graphics3D::GetWindowHeight() {
	return g_windowSystem->GetWindow().GetWndSetting().wndSize.height;
}

void LGF::Graphics3D::SetCamera(const Camera3D& camera) {
	assert(g_camera3DSystem != nullptr);
	g_camera3DSystem->SetCamera(camera);
}

void LGF::Graphics3D::Draw(const Triangle3D& triangle, const Matrix4x4& worldMatrix, const Vector4& color) {
	assert(g_renderer3D != nullptr);
	g_renderer3D->AddTriangle(
		triangle,
		worldMatrix,
		color
	);
}

void LGF::Graphics3D::Draw(const Plane3D& plane, const Matrix4x4& worldMatrix, const Vector4& color) {
	assert(g_renderer3D != nullptr);
	g_renderer3D->AddPlane(
		plane,
		worldMatrix,
		color
	);
}
