#include "SkyboxSystem.h"

#include <algorithm>
#include <cassert>
#include <cmath>

#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/DirectX/ResourceFactory/ResourceFactory.h"
#include "LGF/Graphics/Skybox.h"
#include "LGF/Math/MathConstants.h"

using namespace LGF;

SkyboxSystem::SkyboxSystem(DirectXCore& directX)
	: directX_(directX) {
}

SkyboxSystem::~SkyboxSystem() = default;

bool SkyboxSystem::Initialize() {
	constexpr size_t kConstantBufferAlignment = 256u;
	resource_ = directX_.GetResourceFactory()->CreateUploadBufferResource(kConstantBufferAlignment);
	if (!resource_) {
		return false;
	}

	const HRESULT result = resource_->Map(0, nullptr, reinterpret_cast<void**>(&data_));
	return SUCCEEDED(result);
}

void SkyboxSystem::BeginFrame() {
	pendingData_ = SkyboxForGPU{};
}

void SkyboxSystem::EndFrame() {
	assert(data_);
	*data_ = pendingData_;
}

void SkyboxSystem::Finalize() {
	data_ = nullptr;
	resource_.Reset();
}

void SkyboxSystem::SetSkybox(const Skybox& skybox) {
	assert(skybox.texture_.IsValid());
	assert(!skybox.texture_.isCubeMap_);

	pendingData_.textureIndex = skybox.texture_.srvIndex_;
	pendingData_.intensity = std::isfinite(skybox.intensity)
		? std::max(skybox.intensity, 0.0f)
		: 1.0f;
	pendingData_.rotation = std::isfinite(skybox.rotation)
		? std::remainder(skybox.rotation, Math::TWO_PI)
		: 0.0f;
}

void SkyboxSystem::Bind(ID3D12GraphicsCommandList* commandList, uint32_t rootParamIdx) const {
	assert(commandList);
	assert(resource_);
	commandList->SetGraphicsRootConstantBufferView(
		static_cast<UINT>(rootParamIdx), resource_->GetGPUVirtualAddress());
}
