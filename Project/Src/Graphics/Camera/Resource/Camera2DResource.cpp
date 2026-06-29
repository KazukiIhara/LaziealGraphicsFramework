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
	// リソースの作成
	resource_ = directX_.GetResourceFactory()->CreateUploadBufferResource(sizeof(Camera2DForGPU));
	// マップ
	HRESULT hr = resource_->Map(0, nullptr, reinterpret_cast<void**>(&data_));
	assert(SUCCEEDED(hr));
}

void Camera2DResource::Update(const Camera2D& camera) {

}

void Camera2DResource::Bind(ID3D12GraphicsCommandList* commandList, uint32_t rootParamIdx) {
	assert(resource_);
	commandList->SetGraphicsRootConstantBufferView(static_cast<UINT>(rootParamIdx), resource_->GetGPUVirtualAddress());
}
