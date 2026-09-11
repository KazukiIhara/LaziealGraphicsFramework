#include "DirectionalLightResource.h"

#include <cassert>

#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/DirectX/ResourceFactory/ResourceFactory.h"
#include "LGF/Graphics/DirectionalLight.h"
#include "LGF/Math/MathUtils.h"

using namespace LGF;

DirectionalLightResource::DirectionalLightResource(DirectXCore& directX)
	: directX_(directX) {
}

DirectionalLightResource::~DirectionalLightResource() = default;

void DirectionalLightResource::Initialize() {
	resource_ = directX_.GetResourceFactory()->CreateUploadBufferResource(sizeof(DirectionalLightForGPU));
	assert(resource_);
	const HRESULT hr = resource_->Map(0, nullptr, reinterpret_cast<void**>(&data_));
	assert(SUCCEEDED(hr));
}

void DirectionalLightResource::Update(const DirectionalLight& light) {
	assert(data_);

	constexpr float kDirectionEpsilon = 1.0e-6f;
	data_->direction = Math::LengthSq(light.direction) > kDirectionEpsilon
		? Math::Normalize(light.direction)
		: Vector3{ 0.0f, -1.0f, 0.0f };
	data_->intensity = light.intensity > 0.0f ? light.intensity : 0.0f;
	data_->color = { light.color.x, light.color.y, light.color.z };
	data_->padding = 0.0f;
}

void DirectionalLightResource::Bind(ID3D12GraphicsCommandList* commandList, uint32_t rootParamIdx) {
	assert(commandList);
	assert(resource_);
	commandList->SetGraphicsRootConstantBufferView(
		static_cast<UINT>(rootParamIdx), resource_->GetGPUVirtualAddress());
}
