#include "RTVManager.h"

#include <cassert>

#include "DescriptorHeap/DescriptorHeapFactory.h"
#include "Device/Device.h"

using namespace LGF;

RTVManager::RTVManager(Device& device, DescriptorHeapFactory& descriptorHeapManager)
	: BaseViewManager(device, descriptorHeapManager), device_(device) {
}

void RTVManager::Initialize(uint32_t maxViewCount) {
	BaseViewManager::Initialize(
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		maxViewCount,
		false);
}

void RTVManager::CreateRTVTexture2d(uint32_t rtvIndex, ID3D12Resource* resource, DXGI_FORMAT format) {
	assert(resource);

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = format;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	device_.GetDevice()->CreateRenderTargetView(
		resource,
		&rtvDesc,
		GetCPUDescriptorHandle(rtvIndex));
}
