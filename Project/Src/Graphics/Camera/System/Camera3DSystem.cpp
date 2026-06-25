#include "Camera3DSystem.h"

#include <cassert>

#include "Camera/Camera3D.h"
#include "Camera/Resource/Camera3DResource.h"

using namespace LGF;

Camera3DSystem::Camera3DSystem(DirectXCore& directX) {
	// GPUリソースを生成
	resource_ = std::make_unique<Camera3DResource>(directX);
}

Camera3DSystem::~Camera3DSystem() = default;

void Camera3DSystem::Initialize() {
	// GPUリソース初期化
	resource_->Initialize();
	// デフォルトカメラの生成
	Add("Default", std::make_unique<Camera3D>());

	// 使用するカメラをクリア
	currentCameraName_ = "";
}

void Camera3DSystem::Finalize() {
}

void Camera3DSystem::BeginFrame() {
	// 使用するカメラをクリア
	currentCameraName_ = "";
}

void Camera3DSystem::EndFrame() {
	// 描画前カメラ更新
	for (auto& camera : cameraMap_) {
		camera.second->Update();
	}

	// 使用するカメラを取得
	if (currentCameraName_ == "") {
		currentCameraName_ = "Default";
	}
	Camera3D* currentCam = Get(currentCameraName_);
	if (currentCam == nullptr) {
		currentCam = Get("Default");
	}
	assert(currentCam != nullptr);

	// GPUにバインドするリソースの更新
	resource_->Update(*currentCam);

}

void Camera3DSystem::Add(const std::string& name, std::unique_ptr<Camera3D> camera) {
	assert(camera != nullptr);

	// カメラを追加
	cameraMap_[name] = std::move(camera);
}

void Camera3DSystem::Remove(const std::string& name) {
	// デフォルトカメラは削除しない
	if (name == "Default") {
		return;
	}

	// 使用中なら解除
	if (currentCameraName_ == name) {
		currentCameraName_ = "";
	}

	// カメラを削除
	cameraMap_.erase(name);
}

void Camera3DSystem::Set(const std::string& name) {
	currentCameraName_ = name;
}

Camera3D* Camera3DSystem::Get(const std::string& name) {
	// カメラを検索
	auto it = cameraMap_.find(name);
	if (it == cameraMap_.end()) {
		return nullptr;
	}

	return it->second.get();
}

void Camera3DSystem::Bind(ID3D12GraphicsCommandList* commandList, uint32_t rootParamIdx) {
	resource_->Bind(commandList, rootParamIdx);
}
