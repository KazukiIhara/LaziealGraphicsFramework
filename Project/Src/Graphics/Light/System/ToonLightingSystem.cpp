#include "ToonLightingSystem.h"

#include <stdexcept>

#include "Graphics/Light/Resource/ToonLightingResource.h"

using namespace LGF;

ToonLightingSystem::ToonLightingSystem(DirectXCore& directX)
	: resource_(std::make_unique<ToonLightingResource>(directX)) {
}

ToonLightingSystem::~ToonLightingSystem() = default;

bool ToonLightingSystem::Initialize() {
	resource_->Initialize();
	return true;
}

void ToonLightingSystem::BeginFrame() {
	lighting_ = ToonLighting{};
	lighting_.enabled = false;
	materials_.clear();
	materials_.resize(2u);
}

void ToonLightingSystem::EndFrame() {
	resource_->Update(lighting_, materials_);
}

void ToonLightingSystem::Finalize() {
	resource_.reset();
}

void ToonLightingSystem::SetLighting(const ToonLighting& lighting) {
	lighting_ = lighting;
}

uint32_t ToonLightingSystem::RegisterMaterial(
	ShadingModel3D shadingModel,
	const ToonMaterial3D& toonLighting) {

	if (shadingModel == ShadingModel3D::SceneDefault) {
		return 0u;
	}
	if (shadingModel == ShadingModel3D::PBR) {
		return 1u;
	}

	for (uint32_t index = 2u; index < materials_.size(); ++index) {
		const ToonMaterial3D& material = materials_[index];
		if (material.shadowColor == toonLighting.shadowColor &&
			material.middleColor == toonLighting.middleColor &&
			material.lightColor == toonLighting.lightColor &&
			material.shadowThreshold == toonLighting.shadowThreshold &&
			material.lightThreshold == toonLighting.lightThreshold &&
			material.smoothness == toonLighting.smoothness) {
			return index;
		}
	}
	if (materials_.size() >= ToonLightingResource::MaxMaterialCount) {
		throw std::runtime_error("Too many Toon materials were submitted in one frame.");
	}

	materials_.push_back(toonLighting);
	return static_cast<uint32_t>(materials_.size() - 1u);
}

void ToonLightingSystem::Bind(
	ID3D12GraphicsCommandList* commandList,
	uint32_t lightingRootParamIdx,
	uint32_t materialsRootParamIdx) {

	resource_->Bind(commandList, lightingRootParamIdx, materialsRootParamIdx);
}
