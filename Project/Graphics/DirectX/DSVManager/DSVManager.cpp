#include "DSVManager.h"

#include <cassert>

#include "DescriptorHeap/DescriptorHeapFactory.h"
#include "Device/Device.h"

using namespace LGF;

DSVManager::DSVManager(Device& device, DescriptorHeapFactory& descriptorHeapManager)
	: BaseViewManager(device, descriptorHeapManager), device_(device) {
}

void DSVManager::Initialize(uint32_t maxViewCount) {
	BaseViewManager::Initialize(
		D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
		maxViewCount,
		false);
}

void DSVManager::CreateDSVTexture2d(uint32_t dsvIndex, ID3D12Resource* resource, DXGI_FORMAT format) {
	assert(resource);

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = format;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

	device_.GetDevice()->CreateDepthStencilView(
		resource,
		&dsvDesc,
		GetDescriptorHandleCPU(dsvIndex));
}
