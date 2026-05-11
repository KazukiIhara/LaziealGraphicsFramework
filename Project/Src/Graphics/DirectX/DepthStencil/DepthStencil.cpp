#include "DepthStencil.h"

#include <cassert>

#include "DSVManager/DSVManager.h"
#include "ResourceFactory/ResourceFactory.h"

using namespace LGF;

void DepthStencil::Initialize(
	ResourceFactory& resourceFactory,
	DSVManager& dsvManager,
	uint32_t width,
	uint32_t height,
	DXGI_FORMAT resourceFormat,
	DXGI_FORMAT dsvFormat) {

	resource_ = resourceFactory.CreateDepthStencilTextureResource(
		static_cast<int32_t>(width),
		static_cast<int32_t>(height),
		resourceFormat,
		dsvFormat);

	dsvManager_ = &dsvManager;
	dsvIndex_ = dsvManager_->Allocate();
	dsvManager_->CreateDSVTexture2d(
		dsvIndex_,
		resource_.Get(),
		dsvFormat);
}

ID3D12Resource* DepthStencil::GetResource()const {
	return resource_.Get();
}

uint32_t DepthStencil::GetDSVIndex()const {
	return dsvIndex_;
}

D3D12_CPU_DESCRIPTOR_HANDLE DepthStencil::GetDSVHandleCPU()const {
	assert(dsvManager_);
	return dsvManager_->GetDescriptorHandleCPU(dsvIndex_);
}
