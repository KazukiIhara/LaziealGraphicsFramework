#include "PointLightSystem.h"

#include "Graphics/Light/Resource/PointLightResource.h"

using namespace LGF;

PointLightSystem::PointLightSystem(DirectXCore& directX)
	: resource_(std::make_unique<PointLightResource>(directX)) {
}

PointLightSystem::~PointLightSystem() = default;

bool PointLightSystem::Initialize() {
	resource_->Initialize();
	return true;
}

void PointLightSystem::BeginFrame() {
	light_ = PointLight{ {}, Color::White, 0.0f };
}

void PointLightSystem::EndFrame() {
	resource_->Update(light_);
}

void PointLightSystem::Finalize() {
	resource_.reset();
}

void PointLightSystem::SetLight(const PointLight& light) {
	light_ = light;
}

void PointLightSystem::Bind(ID3D12GraphicsCommandList* commandList, uint32_t rootParamIdx) {
	resource_->Bind(commandList, rootParamIdx);
}
