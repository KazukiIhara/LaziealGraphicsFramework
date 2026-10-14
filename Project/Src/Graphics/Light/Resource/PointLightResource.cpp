#include "PointLightResource.h"

#include <cassert>

#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/DirectX/ResourceFactory/ResourceFactory.h"
#include "Graphics/DirectX/ViewManagers/SRVUAVManager/SRVUAVManager.h"
#include "LGF/Graphics/PointLight.h"

using namespace LGF;

PointLightResource::PointLightResource(DirectXCore& directX)
	: directX_(directX) {
}

PointLightResource::~PointLightResource() = default;

void PointLightResource::Initialize() {
	const bool lightInitialized = lightResources_.Initialize(
		*directX_.GetResourceFactory(),
		sizeof(PointLightForGPU) * PointLight::MaxCount);
	assert(lightInitialized);
	const bool countInitialized = countResources_.Initialize(
		*directX_.GetResourceFactory(),
		sizeof(PointLightCountForGPU));
	assert(countInitialized);

	for (uint32_t frameIndex = 0u; frameIndex < kFrameContextCount; ++frameIndex) {
		const uint32_t srvIndex = directX_.GetResMgr().srvuav->Allocate();
		lightSrvIndices_[frameIndex] = srvIndex;
		directX_.GetResMgr().srvuav->CreateSrvStructuredBuffer(
			srvIndex,
			lightResources_.GetResource(frameIndex),
			PointLight::MaxCount,
			static_cast<UINT>(sizeof(PointLightForGPU)));
	}
}

void PointLightResource::Update(std::span<const PointLight> lights) {
	const uint32_t frameIndex = directX_.GetCurrentFrameContextIndex();
	PointLightForGPU* data = reinterpret_cast<PointLightForGPU*>(
		lightResources_.GetMappedData(frameIndex));
	assert(data);

	for (const PointLight& light : lights) {
		data->position = light.position;
		data->intensity = light.intensity > 0.0f ? light.intensity : 0.0f;
		data->color = { light.color.x, light.color.y, light.color.z };
		data->range = light.range > 0.0f ? light.range : 0.0f;
		++data;
	}

	PointLightCountForGPU* countData = reinterpret_cast<PointLightCountForGPU*>(
		countResources_.GetMappedData(frameIndex));
	assert(countData);
	countData->count = static_cast<uint32_t>(lights.size());
	countData->padding[0] = 0.0f;
	countData->padding[1] = 0.0f;
	countData->padding[2] = 0.0f;
}

void PointLightResource::Bind(
	ID3D12GraphicsCommandList* commandList,
	uint32_t lightsRootParamIdx,
	uint32_t countRootParamIdx) {

	assert(commandList);
	const uint32_t frameIndex = directX_.GetCurrentFrameContextIndex();
	commandList->SetGraphicsRootDescriptorTable(
		static_cast<UINT>(lightsRootParamIdx),
		directX_.GetResMgr().srvuav->GetGPUDescriptorHandle(
			lightSrvIndices_.at(frameIndex)));
	commandList->SetGraphicsRootConstantBufferView(
		static_cast<UINT>(countRootParamIdx),
		countResources_.GetGPUVirtualAddress(frameIndex));
}
