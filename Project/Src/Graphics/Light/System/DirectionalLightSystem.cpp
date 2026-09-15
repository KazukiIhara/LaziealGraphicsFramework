#include "DirectionalLightSystem.h"

#include "Graphics/Light/Resource/DirectionalLightResource.h"

using namespace LGF;

DirectionalLightSystem::DirectionalLightSystem(DirectXCore& directX)
	: resource_(std::make_unique<DirectionalLightResource>(directX)) {
}

DirectionalLightSystem::~DirectionalLightSystem() = default;

bool DirectionalLightSystem::Initialize() {
	resource_->Initialize();
	return true;
}

void DirectionalLightSystem::BeginFrame() {
	light_ = DirectionalLight{};
}

void DirectionalLightSystem::EndFrame() {
	resource_->Update(light_);
}

void DirectionalLightSystem::Finalize() {
	resource_.reset();
}

void DirectionalLightSystem::SetLight(const DirectionalLight& light) {
	light_ = light;
}

const DirectionalLight& DirectionalLightSystem::GetLight() const {
	return light_;
}

void DirectionalLightSystem::Bind(ID3D12GraphicsCommandList* commandList, uint32_t rootParamIdx) {
	resource_->Bind(commandList, rootParamIdx);
}
