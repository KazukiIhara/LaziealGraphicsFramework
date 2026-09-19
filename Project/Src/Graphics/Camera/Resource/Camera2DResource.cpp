#include "Camera2DResource.h"

#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/DirectX/Command/Command.h"
#include "Graphics/DirectX/ResourceFactory/ResourceFactory.h"

#include "LGF/Graphics/Camera2D.h"

#include "LGF/Math/MathUtils.h"

#include <cassert>

using namespace LGF;

Camera2DResource::Camera2DResource(DirectXCore& directX) :
	directX_(directX) {
}

Camera2DResource::~Camera2DResource() = default;

void Camera2DResource::Initialize() {
	const bool initialized = resources_.Initialize(
		*directX_.GetResourceFactory(),
		sizeof(Camera2DForGPU));
	assert(initialized);
}

void Camera2DResource::Update(const Camera2D& camera) {
	Camera2DForGPU* data_ = reinterpret_cast<Camera2DForGPU*>(
		resources_.GetMappedData(directX_.GetCurrentFrameContextIndex()));
	assert(data_);

	// カメラのワールド行列を作る
	const Matrix3x3 worldMatrix =
		Math::MakeAffineMatrix(
			{ 1.0f, 1.0f },
			camera.rotation,
			camera.translation);

	// View行列を作る
	const Matrix3x3 viewMatrix = Math::Inverse(worldMatrix);

	// Projection行列を作る
	const Matrix3x3 projectionMatrix =
		Math::MakeOrthographicMatrix(
			0.0f,
			0.0f,
			camera.size.x,
			camera.size.y);

	// 4x4行列に変換
	const Matrix3x3 viewProjection = viewMatrix * projectionMatrix;

	data_->viewProjection = Math::ConvertMatrix4x4(viewProjection);
}

void Camera2DResource::Bind(ID3D12GraphicsCommandList* commandList, uint32_t rootParamIdx) {
	commandList->SetGraphicsRootConstantBufferView(
		static_cast<UINT>(rootParamIdx),
		resources_.GetGPUVirtualAddress(directX_.GetCurrentFrameContextIndex()));
}
