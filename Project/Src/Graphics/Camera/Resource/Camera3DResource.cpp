#include "Camera3DResource.h"

#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/DirectX/Command/Command.h"
#include "Graphics/DirectX/ResourceFactory/ResourceFactory.h"

#include "Graphics/Camera/Camera3D.h"

#include "Math/Functions/MatrixMath.h"

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
	data_->viewProjection = camera.GetViewMatrix() * camera.GetProjectionMatrix();
	data_->worldPosition = camera.GetTranslation();
}

void Camera3DResource::Bind(ID3D12GraphicsCommandList* commandList, uint32_t rootParamIdx) {
	assert(resource_);
	commandList->SetGraphicsRootConstantBufferView(static_cast<UINT>(rootParamIdx), resource_->GetGPUVirtualAddress());
}
