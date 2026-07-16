#include "RenderTexture.h"

#include <cassert>

#include "Graphics/DirectX/ResourceFactory/ResourceFactory.h"
#include "Graphics/DirectX/ViewManagers/RTVManager/RTVManager.h"
#include "Graphics/DirectX/ViewManagers/SRVUAVManager/SRVUAVManager.h"

using namespace LGF;

RenderTexture::RenderTexture(
	ResourceFactory& resourceFactory,
	RTVManager& rtvManager,
	SRVUAVManager& srvUavManager) :
	resourceFactory_(resourceFactory),
	rtvManager_(rtvManager),
	srvUavManager_(srvUavManager) {
}

RenderTexture::~RenderTexture() = default;

bool RenderTexture::Initialize(const RenderTextureDesc& desc) {
	if (desc.width == 0u || desc.height == 0u) {
		return false;
	}

	desc_ = desc;
	return CreateResourceAndViews();
}

bool RenderTexture::Resize(uint32_t width, uint32_t height) {
	if (width == 0u || height == 0u) {
		return false;
	}

	if (desc_.width == width && desc_.height == height) {
		return true;
	}

	desc_.width = width;
	desc_.height = height;

	return CreateResourceAndViews();
}

void RenderTexture::Transition(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES nextState) {
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

void RenderTexture::Clear(ID3D12GraphicsCommandList* commandList) {
	assert(commandList);

	const float clearColor[] = {
		desc_.clearColor.x,
		desc_.clearColor.y,
		desc_.clearColor.z,
		desc_.clearColor.w
	};

	commandList->ClearRenderTargetView(GetRTVHandleCPU(), clearColor, 0, nullptr);
}

ID3D12Resource* RenderTexture::GetResource() const {
	return resource_.Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE RenderTexture::GetRTVHandleCPU() const {
	return rtvManager_.GetCPUDescriptorHandle(rtvIndex_);
}

D3D12_GPU_DESCRIPTOR_HANDLE RenderTexture::GetSRVHandleGPU() const {
	return srvUavManager_.GetGPUDescriptorHandle(srvIndex_);
}

uint32_t RenderTexture::GetWidth() const {
	return desc_.width;
}

uint32_t RenderTexture::GetHeight() const {
	return desc_.height;
}

const Vector4& RenderTexture::GetClearColor() const {
	return desc_.clearColor;
}

D3D12_RESOURCE_STATES RenderTexture::GetCurrentState() const {
	return currentState_;
}

bool RenderTexture::CreateResourceAndViews() {
	if (!isViewAllocated_) {
		rtvIndex_ = rtvManager_.Allocate();
		srvIndex_ = srvUavManager_.Allocate();
		isViewAllocated_ = true;
	}

	resource_ = resourceFactory_.CreateRenderTargetTextureResource(
		static_cast<int32_t>(desc_.width),
		static_cast<int32_t>(desc_.height),
		desc_.resourceFormat,
		desc_.rtvFormat,
		desc_.clearColor);

	if (!resource_) {
		return false;
	}

	rtvManager_.CreateRTVTexture2d(rtvIndex_, resource_.Get(), desc_.rtvFormat);
	srvUavManager_.CreateSrvTexture2d(srvIndex_, resource_.Get(), desc_.srvFormat, 1u);

	currentState_ = D3D12_RESOURCE_STATE_RENDER_TARGET;
	return true;
}
