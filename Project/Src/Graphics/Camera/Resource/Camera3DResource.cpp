#include "Camera3DResource.h"

#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/DirectX/Command/Command.h"
#include "Graphics/DirectX/ResourceFactory/ResourceFactory.h"

#include "LGF/Graphics/Camera3D.h"

#include "LGF/Math/MathUtils.h"

#include <cassert>

using namespace LGF;

Camera3DResource::Camera3DResource(DirectXCore& directX) :
	directX_(directX) {
}

Camera3DResource::~Camera3DResource() = default;

void Camera3DResource::Initialize() {
	// リソースの作成
	resource_ = directX_.GetResourceFactory()->CreateUploadBufferResource(sizeof(Camera3DForGPU));
	// マップ
	HRESULT hr = resource_->Map(0, nullptr, reinterpret_cast<void**>(&data_));
	assert(SUCCEEDED(hr));
}

void Camera3DResource::Update(const Camera3D& camera) {
	assert(data_);

	// 回転を作る
	const Quaternion rotation_ = Math::MakeQuaternion({
		camera.pitch,
		camera.yaw,
		0.0f
		});

	// カメラのワールド行列を作る
	const Matrix4x4 worldMatrix =
		Math::MakeAffineMatrix(
			{ 1.0f, 1.0f, 1.0f },
			rotation_,
			camera.translation);

	// View行列を作る
	const Matrix4x4 viewMatrix = Math::Inverse(worldMatrix);

	// Projection行列を作る
	const Matrix4x4 projectionMatrix =
		Math::MakePerspectiveFovMatrix(
			camera.param.fovY,
			camera.param.aspectRatio,
			camera.param.nearClip,
			camera.param.farClip);

	data_->viewProjection = viewMatrix * projectionMatrix;
	data_->worldPosition = camera.translation;
}

void Camera3DResource::Bind(ID3D12GraphicsCommandList* commandList, uint32_t rootParamIdx) {
	assert(resource_);
	commandList->SetGraphicsRootConstantBufferView(static_cast<UINT>(rootParamIdx), resource_->GetGPUVirtualAddress());
}
