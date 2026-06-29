#include "Camera2DSystem.h"

#include <cassert>

#include "Graphics/Camera/Resource/Camera2DResource.h"

using namespace LGF;

Camera2DSystem::Camera2DSystem(DirectXCore& directX) {
	// GPUリソースを生成
	resource_ = std::make_unique<Camera2DResource>(directX);
}

Camera2DSystem::~Camera2DSystem() = default;

bool Camera2DSystem::Initialize() {
	// GPUリソース初期化
	resource_->Initialize();

	return false;
}

void Camera2DSystem::BeginFrame() {
	// カメラ初期化
	camera_ = Camera2D();
}

void Camera2DSystem::EndFrame() {
	// GPUにバインドするリソースの更新
	resource_->Update(camera_);
}

void Camera2DSystem::Finalize() {
	resource_.reset();
}

void Camera2DSystem::SetCamera(const Camera2D& camera) {
	camera_ = camera;
}

void Camera2DSystem::Bind(ID3D12GraphicsCommandList* commandList, uint32_t rootParamIdx) {
	resource_->Bind(commandList, rootParamIdx);
}
