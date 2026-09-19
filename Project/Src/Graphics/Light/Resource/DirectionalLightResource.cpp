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
	const bool initialized = resources_.Initialize(
		*directX_.GetResourceFactory(),
		sizeof(DirectionalLightForGPU));
	assert(initialized);
}

void DirectionalLightResource::Update(const DirectionalLight& light) {
	DirectionalLightForGPU* data_ = reinterpret_cast<DirectionalLightForGPU*>(
		resources_.GetMappedData(directX_.GetCurrentFrameContextIndex()));
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
	commandList->SetGraphicsRootConstantBufferView(
		static_cast<UINT>(rootParamIdx),
		resources_.GetGPUVirtualAddress(directX_.GetCurrentFrameContextIndex()));
}
