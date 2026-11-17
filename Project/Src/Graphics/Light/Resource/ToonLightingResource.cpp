#include "ToonLightingResource.h"

#include <algorithm>
#include <cassert>

#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/DirectX/ResourceFactory/ResourceFactory.h"
#include "Graphics/DirectX/ViewManagers/SRVUAVManager/SRVUAVManager.h"
#include "LGF/Graphics/Material.h"
#include "LGF/Graphics/ToonLighting.h"

using namespace LGF;

ToonLightingResource::ToonLightingResource(DirectXCore& directX)
	: directX_(directX) {
}

ToonLightingResource::~ToonLightingResource() = default;

void ToonLightingResource::Initialize() {
	const bool initialized = resources_.Initialize(
		*directX_.GetResourceFactory(),
		sizeof(ToonLightingForGPU));
	assert(initialized);
	const bool materialsInitialized = materialResources_.Initialize(
		*directX_.GetResourceFactory(),
		sizeof(ToonMaterialForGPU) * MaxMaterialCount);
	assert(materialsInitialized);

	for (uint32_t frameIndex = 0u; frameIndex < kFrameContextCount; ++frameIndex) {
		const uint32_t srvIndex = directX_.GetResMgr().srvuav->Allocate();
		materialSrvIndices_[frameIndex] = srvIndex;
		directX_.GetResMgr().srvuav->CreateSrvStructuredBuffer(
			srvIndex,
			materialResources_.GetResource(frameIndex),
			MaxMaterialCount,
			static_cast<UINT>(sizeof(ToonMaterialForGPU)));
	}
}

void ToonLightingResource::Update(
	const ToonLighting& lighting,
	std::span<const ToonMaterial3D> materials) {

	const uint32_t frameIndex = directX_.GetCurrentFrameContextIndex();
	ToonLightingForGPU* data = reinterpret_cast<ToonLightingForGPU*>(
		resources_.GetMappedData(frameIndex));
	assert(data);

	data->shadowColor = {
		std::max(lighting.shadowColor.x, 0.0f),
		std::max(lighting.shadowColor.y, 0.0f),
		std::max(lighting.shadowColor.z, 0.0f),
	};
	data->enabled = lighting.enabled ? 1u : 0u;
	data->middleColor = {
		std::max(lighting.middleColor.x, 0.0f),
		std::max(lighting.middleColor.y, 0.0f),
		std::max(lighting.middleColor.z, 0.0f),
	};
	data->shadowThreshold = std::clamp(lighting.shadowThreshold, 0.0f, 1.0f);
	data->lightColor = {
		std::max(lighting.lightColor.x, 0.0f),
		std::max(lighting.lightColor.y, 0.0f),
		std::max(lighting.lightColor.z, 0.0f),
	};
	data->lightThreshold = std::clamp(
		lighting.lightThreshold,
		data->shadowThreshold,
		1.0f);
	data->smoothness = std::clamp(lighting.smoothness, 0.0f, 0.5f);
	data->padding[0] = 0.0f;
	data->padding[1] = 0.0f;
	data->padding[2] = 0.0f;

	ToonMaterialForGPU* materialData = reinterpret_cast<ToonMaterialForGPU*>(
		materialResources_.GetMappedData(frameIndex));
	assert(materialData);
	for (const ToonMaterial3D& material : materials) {
		materialData->shadowColor = {
			std::max(material.shadowColor.x, 0.0f),
			std::max(material.shadowColor.y, 0.0f),
			std::max(material.shadowColor.z, 0.0f),
		};
		materialData->shadowThreshold = std::clamp(material.shadowThreshold, 0.0f, 1.0f);
		materialData->middleColor = {
			std::max(material.middleColor.x, 0.0f),
			std::max(material.middleColor.y, 0.0f),
			std::max(material.middleColor.z, 0.0f),
		};
		materialData->lightThreshold = std::clamp(
			material.lightThreshold,
			materialData->shadowThreshold,
			1.0f);
		materialData->lightColor = {
			std::max(material.lightColor.x, 0.0f),
			std::max(material.lightColor.y, 0.0f),
			std::max(material.lightColor.z, 0.0f),
		};
		materialData->smoothness = std::clamp(material.smoothness, 0.0f, 0.5f);
		materialData->padding[0] = 0.0f;
		materialData->padding[1] = 0.0f;
		materialData->padding[2] = 0.0f;
		materialData->padding[3] = 0.0f;
		++materialData;
	}
}

void ToonLightingResource::Bind(
	ID3D12GraphicsCommandList* commandList,
	uint32_t lightingRootParamIdx,
	uint32_t materialsRootParamIdx) {

	assert(commandList);
	const uint32_t frameIndex = directX_.GetCurrentFrameContextIndex();
	commandList->SetGraphicsRootConstantBufferView(
		static_cast<UINT>(lightingRootParamIdx),
		resources_.GetGPUVirtualAddress(frameIndex));
	commandList->SetGraphicsRootDescriptorTable(
		static_cast<UINT>(materialsRootParamIdx),
		directX_.GetResMgr().srvuav->GetGPUDescriptorHandle(
			materialSrvIndices_.at(frameIndex)));
}
