#include "PointLightResource.h"

#include <cassert>

#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/DirectX/ResourceFactory/ResourceFactory.h"
#include "LGF/Graphics/PointLight.h"

using namespace LGF;

PointLightResource::PointLightResource(DirectXCore& directX)
	: directX_(directX) {
}

PointLightResource::~PointLightResource() = default;

void PointLightResource::Initialize() {
	const bool initialized = resources_.Initialize(
		*directX_.GetResourceFactory(),
		sizeof(PointLightForGPU));
	assert(initialized);
}

void PointLightResource::Update(const PointLight& light) {
	PointLightForGPU* data_ = reinterpret_cast<PointLightForGPU*>(
		resources_.GetMappedData(directX_.GetCurrentFrameContextIndex()));
	assert(data_);

	data_->position = light.position;
	data_->intensity = light.intensity > 0.0f ? light.intensity : 0.0f;
	data_->color = { light.color.x, light.color.y, light.color.z };
	data_->range = light.range > 0.0f ? light.range : 0.0f;
}

void PointLightResource::Bind(ID3D12GraphicsCommandList* commandList, uint32_t rootParamIdx) {
	assert(commandList);
	commandList->SetGraphicsRootConstantBufferView(
		static_cast<UINT>(rootParamIdx),
		resources_.GetGPUVirtualAddress(directX_.GetCurrentFrameContextIndex()));
}
