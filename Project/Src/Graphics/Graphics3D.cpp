#include "LGF/Graphics/Graphics3D.h"

#include <cassert>

#include "Graphics3DInternal.h"
#include "Core/Window/System/WindowSystem.h"
#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/Camera/System/Camera3DSystem.h"
#include "Graphics/Light/System/DirectionalLightSystem.h"
#include "Graphics/Renderer/Renderer3D.h"
#include "LGF/Graphics/Model.h"

namespace {
	LGF::DirectXCore* g_directX = nullptr;
	LGF::WindowSystem* g_windowSystem = nullptr;
	LGF::Camera3DSystem* g_camera3DSystem = nullptr;
	LGF::DirectionalLightSystem* g_directionalLightSystem = nullptr;
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

void LGF::Graphics3D::Internal::SetDirectXCore(DirectXCore* directX) {
	assert(directX != nullptr);
	g_directX = directX;
}

void LGF::Graphics3D::Internal::ResetDirectXCore() {
	g_directX = nullptr;
}

void LGF::Graphics3D::Internal::SetDirectionalLightSystem(DirectionalLightSystem* lightSystem) {
	assert(lightSystem != nullptr);
	g_directionalLightSystem = lightSystem;
}

void LGF::Graphics3D::Internal::ResetDirectionalLightSystem() {
	g_directionalLightSystem = nullptr;
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

void LGF::Graphics3D::SetCullingCamera(const Camera3D& camera) {
	assert(g_camera3DSystem != nullptr);
	g_camera3DSystem->SetCullingCamera(camera);
}

void LGF::Graphics3D::SetDirectionalLight(const DirectionalLight& light) {
	assert(g_directionalLightSystem != nullptr);
	g_directionalLightSystem->SetLight(light);
}

void LGF::Graphics3D::SetExposure(float exposureEV) {
	assert(g_directX != nullptr);
	g_directX->SetExposure(exposureEV);
}

void LGF::Graphics3D::Draw(const Triangle3D& triangle, const Matrix4x4& worldMatrix, const Vector4& color) {
	assert(g_renderer3D != nullptr);
	g_renderer3D->AddTriangle(
		triangle,
		worldMatrix,
		Material3D{ .color = color }
	);
}

void LGF::Graphics3D::Draw(const Box3D& box, const Matrix4x4& worldMatrix, const Vector4& color) {
	assert(g_renderer3D != nullptr);
	g_renderer3D->AddBox(
		box,
		worldMatrix,
		Material3D{ .color = color }
	);
}

void LGF::Graphics3D::Draw(const Capsule3D& capsule, const Matrix4x4& worldMatrix, const Vector4& color) {
	assert(g_renderer3D != nullptr);
	g_renderer3D->AddCapsule(
		capsule,
		worldMatrix,
		Material3D{ .color = color }
	);
}

void LGF::Graphics3D::Draw(const Cone3D& cone, const Matrix4x4& worldMatrix, const Vector4& color) {
	assert(g_renderer3D != nullptr);
	g_renderer3D->AddCone(
		cone,
		worldMatrix,
		Material3D{ .color = color }
	);
}

void LGF::Graphics3D::Draw(const Cylinder3D& cylinder, const Matrix4x4& worldMatrix, const Vector4& color) {
	assert(g_renderer3D != nullptr);
	g_renderer3D->AddCylinder(
		cylinder,
		worldMatrix,
		Material3D{ .color = color }
	);
}

void LGF::Graphics3D::Draw(const Line3D& line, const Matrix4x4& worldMatrix, const Vector4& color) {
	assert(g_renderer3D != nullptr);
	g_renderer3D->AddLine(
		line,
		worldMatrix,
		Material3D{ .color = color, .unlit = true }
	);
}

void LGF::Graphics3D::Draw(const Plane3D& plane, const Matrix4x4& worldMatrix, const Vector4& color) {
	assert(g_renderer3D != nullptr);
	g_renderer3D->AddPlane(
		plane,
		worldMatrix,
		Material3D{ .color = color }
	);
}

void LGF::Graphics3D::Draw(const Ground3D& ground, const Matrix4x4& worldMatrix, const Vector4& color) {
	assert(g_renderer3D != nullptr);
	g_renderer3D->AddGround(
		ground,
		worldMatrix,
		Material3D{ .color = color }
	);
}

void LGF::Graphics3D::Draw(const Sphere3D& sphere, const Matrix4x4& worldMatrix, const Vector4& color) {
	assert(g_renderer3D != nullptr);
	g_renderer3D->AddSphere(
		sphere,
		worldMatrix,
		Material3D{ .color = color }
	);
}

void LGF::Graphics3D::Draw(
	const Model& model,
	const Matrix4x4& worldMatrix,
	const Vector4& color,
	const ModelDrawOptions& options) {

	assert(g_renderer3D != nullptr);
	g_renderer3D->AddModel(model, worldMatrix, color, options);
}

void LGF::Graphics3D::Draw(const Triangle3D& shape, const Matrix4x4& worldMatrix, const Material3D& material) {
	assert(g_renderer3D != nullptr);
	g_renderer3D->AddTriangle(shape, worldMatrix, material);
}

void LGF::Graphics3D::Draw(const Box3D& shape, const Matrix4x4& worldMatrix, const Material3D& material) {
	assert(g_renderer3D != nullptr);
	g_renderer3D->AddBox(shape, worldMatrix, material);
}

void LGF::Graphics3D::Draw(const Capsule3D& shape, const Matrix4x4& worldMatrix, const Material3D& material) {
	assert(g_renderer3D != nullptr);
	g_renderer3D->AddCapsule(shape, worldMatrix, material);
}

void LGF::Graphics3D::Draw(const Cone3D& shape, const Matrix4x4& worldMatrix, const Material3D& material) {
	assert(g_renderer3D != nullptr);
	g_renderer3D->AddCone(shape, worldMatrix, material);
}

void LGF::Graphics3D::Draw(const Cylinder3D& shape, const Matrix4x4& worldMatrix, const Material3D& material) {
	assert(g_renderer3D != nullptr);
	g_renderer3D->AddCylinder(shape, worldMatrix, material);
}

void LGF::Graphics3D::Draw(const Line3D& shape, const Matrix4x4& worldMatrix, const Material3D& material) {
	assert(g_renderer3D != nullptr);
	g_renderer3D->AddLine(shape, worldMatrix, material);
}

void LGF::Graphics3D::Draw(const Plane3D& shape, const Matrix4x4& worldMatrix, const Material3D& material) {
	assert(g_renderer3D != nullptr);
	g_renderer3D->AddPlane(shape, worldMatrix, material);
}

void LGF::Graphics3D::Draw(const Ground3D& shape, const Matrix4x4& worldMatrix, const Material3D& material) {
	assert(g_renderer3D != nullptr);
	g_renderer3D->AddGround(shape, worldMatrix, material);
}

void LGF::Graphics3D::Draw(const Sphere3D& shape, const Matrix4x4& worldMatrix, const Material3D& material) {
	assert(g_renderer3D != nullptr);
	g_renderer3D->AddSphere(shape, worldMatrix, material);
}
