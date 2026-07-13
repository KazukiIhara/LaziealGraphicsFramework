#include "DepthStencil.h"

#include <cassert>

#include "Graphics/DirectX/ViewManagers/DSVManager/DSVManager.h"
#include "Graphics/DirectX/ResourceFactory/ResourceFactory.h"

using namespace LGF;

DepthStencil::DepthStencil(ResourceFactory& resourceFactory, DSVManager& dsvManager)
	:resourceFactory_(resourceFactory), dsvManager_(dsvManager) {
}

void DepthStencil::Initialize(uint32_t width, uint32_t height, DXGI_FORMAT resourceFormat, DXGI_FORMAT dsvFormat) {
	resourceFormat_ = resourceFormat;
	dsvFormat_ = dsvFormat;

	if (!isDsvAllocated_) {
		dsvIndex_ = dsvManager_.Allocate();
		isDsvAllocated_ = true;
	}

	Resize(width, height);
}

void DepthStencil::Resize(uint32_t width, uint32_t height) {
	assert(width != 0 && height != 0);

	resource_.Reset();
	resource_ = resourceFactory_.CreateDepthStencilTextureResource(
		static_cast<int32_t>(width),
		static_cast<int32_t>(height),
		resourceFormat_,
		dsvFormat_);

	dsvManager_.CreateDSVTexture2d(
		dsvIndex_,
		resource_.Get(),
		dsvFormat_);
}

ID3D12Resource* DepthStencil::GetResource()const {
	return resource_.Get();
}

uint32_t DepthStencil::GetDSVIndex()const {
	return dsvIndex_;
}

D3D12_CPU_DESCRIPTOR_HANDLE DepthStencil::GetDSVHandleCPU()const {
	return dsvManager_.GetCPUDescriptorHandle(dsvIndex_);
}
