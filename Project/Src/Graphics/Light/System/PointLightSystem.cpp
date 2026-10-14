#include "PointLightSystem.h"

#include <stdexcept>

#include "Graphics/Light/Resource/PointLightResource.h"

using namespace LGF;

PointLightSystem::PointLightSystem(DirectXCore& directX)
	: resource_(std::make_unique<PointLightResource>(directX)) {
}

PointLightSystem::~PointLightSystem() = default;

bool PointLightSystem::Initialize() {
	resource_->Initialize();
	lights_.reserve(PointLight::MaxCount);
	return true;
}

void PointLightSystem::BeginFrame() {
	lights_.clear();
}

void PointLightSystem::EndFrame() {
	resource_->Update(lights_);
}

void PointLightSystem::Finalize() {
	resource_.reset();
}

void PointLightSystem::SetLight(const PointLight& light) {
	if (lights_.size() >= PointLight::MaxCount) {
		throw std::runtime_error("Too many point lights were submitted in one frame.");
	}
	lights_.push_back(light);
}

void PointLightSystem::Bind(
	ID3D12GraphicsCommandList* commandList,
	uint32_t lightsRootParamIdx,
	uint32_t countRootParamIdx) {

	resource_->Bind(commandList, lightsRootParamIdx, countRootParamIdx);
}
