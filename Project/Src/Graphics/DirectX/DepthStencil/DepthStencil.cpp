#include "DepthStencil.h"

#include <cassert>

#include "Graphics/DirectX/ViewManagers/DSVManager/DSVManager.h"
#include "Graphics/DirectX/ViewManagers/SRVUAVManager/SRVUAVManager.h"
#include "Graphics/DirectX/ResourceFactory/ResourceFactory.h"

using namespace LGF;

DepthStencil::DepthStencil(ResourceFactory& resourceFactory, DSVManager& dsvManager, SRVUAVManager& srvUavManager)
	:resourceFactory_(resourceFactory), dsvManager_(dsvManager), srvUavManager_(srvUavManager) {
}

void DepthStencil::Initialize(uint32_t width, uint32_t height, DXGI_FORMAT resourceFormat, DXGI_FORMAT dsvFormat, DXGI_FORMAT srvFormat) {
	resourceFormat_ = resourceFormat;
	dsvFormat_ = dsvFormat;
	srvFormat_ = srvFormat;

	if (!isDsvAllocated_) {
		dsvIndex_ = dsvManager_.Allocate();
		isDsvAllocated_ = true;
	}
	if (!isSrvAllocated_) {
		srvIndex_ = srvUavManager_.Allocate();
		isSrvAllocated_ = true;
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

	srvUavManager_.CreateSrvTexture2d(
		srvIndex_,
		resource_.Get(),
		srvFormat_,
		1u);

	currentState_ = D3D12_RESOURCE_STATE_DEPTH_WRITE;
}

void DepthStencil::Transition(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES nextState) {
	assert(commandList);
	assert(resource_);

	if (currentState_ == nextState) {
		return;
	}

	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = resource_.Get();
	barrier.Transition.StateBefore = currentState_;
	barrier.Transition.StateAfter = nextState;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	commandList->ResourceBarrier(1, &barrier);
	currentState_ = nextState;
}

ID3D12Resource* DepthStencil::GetResource()const {
	return resource_.Get();
}

uint32_t DepthStencil::GetDSVIndex()const {
	return dsvIndex_;
}

uint32_t DepthStencil::GetSRVIndex()const {
	return srvIndex_;
}

D3D12_CPU_DESCRIPTOR_HANDLE DepthStencil::GetDSVHandleCPU()const {
	return dsvManager_.GetCPUDescriptorHandle(dsvIndex_);
}

D3D12_GPU_DESCRIPTOR_HANDLE DepthStencil::GetSRVHandleGPU()const {
	return srvUavManager_.GetGPUDescriptorHandle(srvIndex_);
}
