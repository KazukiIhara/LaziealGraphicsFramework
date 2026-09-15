#include "Camera3DResource.h"

#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/DirectX/Command/Command.h"
#include "Graphics/DirectX/ResourceFactory/ResourceFactory.h"

#include "LGF/Graphics/Camera3D.h"
#include "LGF/Graphics/Graphics3D.h"

#include "LGF/Math/MathUtils.h"

#include <cassert>
#include <cmath>

using namespace LGF;

namespace {

	Vector4 NormalizePlane(const Vector4& plane) {
		const float length = std::sqrt(
			plane.x * plane.x +
			plane.y * plane.y +
			plane.z * plane.z);
		assert(length > Math::EPSILON);
		return plane / length;
	}

	void ExtractFrustumPlanes(const Matrix4x4& matrix, Vector4 (&planes)[6]) {
		// 行ベクトル規約なので、クリップ空間の各不等式を行列の列から作る。
		planes[0] = NormalizePlane({
			matrix.m[0][3] + matrix.m[0][0],
			matrix.m[1][3] + matrix.m[1][0],
			matrix.m[2][3] + matrix.m[2][0],
			matrix.m[3][3] + matrix.m[3][0] });
		planes[1] = NormalizePlane({
			matrix.m[0][3] - matrix.m[0][0],
			matrix.m[1][3] - matrix.m[1][0],
			matrix.m[2][3] - matrix.m[2][0],
			matrix.m[3][3] - matrix.m[3][0] });
		planes[2] = NormalizePlane({
			matrix.m[0][3] + matrix.m[0][1],
			matrix.m[1][3] + matrix.m[1][1],
			matrix.m[2][3] + matrix.m[2][1],
			matrix.m[3][3] + matrix.m[3][1] });
		planes[3] = NormalizePlane({
			matrix.m[0][3] - matrix.m[0][1],
			matrix.m[1][3] - matrix.m[1][1],
			matrix.m[2][3] - matrix.m[2][1],
			matrix.m[3][3] - matrix.m[3][1] });
		planes[4] = NormalizePlane({
			matrix.m[0][2],
			matrix.m[1][2],
			matrix.m[2][2],
			matrix.m[3][2] });
		planes[5] = NormalizePlane({
			matrix.m[0][3] - matrix.m[0][2],
			matrix.m[1][3] - matrix.m[1][2],
			matrix.m[2][3] - matrix.m[2][2],
			matrix.m[3][3] - matrix.m[3][2] });
	}

}

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
	const Quaternion rotation_ =
		Math::MakeQuaternion(Vector3(camera.pitch, camera.yaw, 0.0f));

	// カメラのワールド行列を作る
	const Matrix4x4 worldMatrix =
		Math::MakeAffineMatrix(
			{ 1.0f, 1.0f, 1.0f },
			rotation_,
			camera.translation);

	// View行列を作る
	const Matrix4x4 viewMatrix = Math::Inverse(worldMatrix);

	// Projection行列を作る
	const float aspectRatio =
		camera.param.useWindowAspectRatio
		? static_cast<float>(Graphics3D::GetWindowWidth()) / static_cast<float>(Graphics3D::GetWindowHeight())
		: camera.param.aspectRatio;

	const Matrix4x4 projectionMatrix =
		Math::MakePerspectiveFovMatrix(
			camera.param.fovY,
			aspectRatio,
			camera.param.nearClip,
			camera.param.farClip);

	// データ書き込み
	data_->viewProjection = viewMatrix * projectionMatrix;
	data_->inverseViewProjection = Math::Inverse(data_->viewProjection);
	data_->worldPosition = camera.translation;
	ExtractFrustumPlanes(data_->viewProjection, data_->frustumPlanes);

}

void Camera3DResource::Bind(ID3D12GraphicsCommandList* commandList, uint32_t rootParamIdx) {
	assert(resource_);
	commandList->SetGraphicsRootConstantBufferView(static_cast<UINT>(rootParamIdx), resource_->GetGPUVirtualAddress());
}
