#include "Camera3DSystem.h"

#include <cassert>

#include "Graphics/Camera/Resource/Camera3DResource.h"

using namespace LGF;

Camera3DSystem::Camera3DSystem(DirectXCore& directX) {
	// GPUリソースを生成
	resource_ = std::make_unique<Camera3DResource>(directX);
	cullingResource_ = std::make_unique<Camera3DResource>(directX);
}

Camera3DSystem::~Camera3DSystem() = default;

bool Camera3DSystem::Initialize() {
	// GPUリソース初期化
	resource_->Initialize();
	cullingResource_->Initialize();

	return true;
}

void Camera3DSystem::BeginFrame() {
	// カメラ初期化
	camera_ = Camera3D();
	cullingCamera_.reset();
	shadowCamera_.reset();
}

void Camera3DSystem::EndFrame() {
	// GPUにバインドするリソースの更新
	resource_->Update(camera_);
	cullingResource_->Update(cullingCamera_.value_or(camera_));
}

void Camera3DSystem::Finalize() {
	resource_.reset();
	cullingResource_.reset();
}

void Camera3DSystem::SetCamera(const Camera3D& camera) {
	camera_ = camera;
}

void Camera3DSystem::SetCullingCamera(const Camera3D& camera) {
	cullingCamera_ = camera;
}

void Camera3DSystem::SetShadowCamera(const Camera3D& camera) {
	shadowCamera_ = camera;
}

const Camera3D& Camera3DSystem::GetShadowCamera() const {
	return shadowCamera_ ? *shadowCamera_ : camera_;
}

void Camera3DSystem::Bind(ID3D12GraphicsCommandList* commandList, uint32_t rootParamIdx) {
	resource_->Bind(commandList, rootParamIdx);
}

void Camera3DSystem::BindCulling(ID3D12GraphicsCommandList* commandList, uint32_t rootParamIdx) {
	cullingResource_->Bind(commandList, rootParamIdx);
}
