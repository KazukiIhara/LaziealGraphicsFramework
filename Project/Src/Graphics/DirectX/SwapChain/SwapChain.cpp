#include "SwapChain.h"

#include <cassert>

#include "Graphics/DirectX/Device/Device.h"
#include "Graphics/DirectX/Command/Command.h"
#include "Graphics/DirectX/ViewManagers/RTVManager/RTVManager.h"
#include "Graphics/DirectX/Viewport/Viewport.h"
#include "Graphics/DirectX/ScissorRect/ScissorRect.h"

using namespace LGF;

SwapChain::SwapChain(
	void* windowHandle,
	uint32_t width,
	uint32_t height,
	Device& device,
	Command& cmd,
	RTVManager& rtvMgr,
	Viewport& viewport,
	ScissorRect& scissorRect) :
	windowHandle_(windowHandle),
	device_(device),
	cmd_(cmd),
	rtvMgr_(rtvMgr),
	viewport_(viewport),
	scissorRect_(scissorRect) {
	swapChainDesc_.Width = width;
	swapChainDesc_.Height = height;
}

SwapChain::~SwapChain() {
}

bool SwapChain::Initialize() {
	// スワップチェーン作成
	bool checkFlag = CreateSwapChain();
	// リソースを作成
	checkFlag = CreateResources();
	// RTVを作成
	CreateRTV();

	return checkFlag;
}

bool SwapChain::Resize(uint32_t width, uint32_t height) {
	if (width == 0 || height == 0) {
		return false;
	}

	swapChainDesc_.Width = width;
	swapChainDesc_.Height = height;

	for (ComPtr<ID3D12Resource>& resource : swapChainResources_) {
		resource.Reset();
	}

	HRESULT hr = swapChain_->ResizeBuffers(
		swapChainDesc_.BufferCount,
		swapChainDesc_.Width,
		swapChainDesc_.Height,
		swapChainDesc_.Format,
		swapChainDesc_.Flags
	);
	if (FAILED(hr)) {
		assert(false);
		return false;
	}

	if (!CreateResources()) {
		return false;
	}

	CreateRTV();
	return true;
}

void SwapChain::BeginFrame() {
	backBufferIndex_ = swapChain_->GetCurrentBackBufferIndex();
	TransitionToRenderTarget();
}

void SwapChain::EndFrame() {
	TransitionToPresent();
}

void SwapChain::Present() {
	swapChain_->Present(1, 0);
}

void SwapChain::TransitionToRenderTarget() {
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = GetCurrentBackBufferResource();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	cmd_.GetList()->ResourceBarrier(1, &barrier);
}

void SwapChain::TransitionToPresent() {
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = GetCurrentBackBufferResource();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	cmd_.GetList()->ResourceBarrier(1, &barrier);
}

ID3D12Resource* SwapChain::GetCurrentBackBufferResource() {
	backBufferIndex_ = swapChain_->GetCurrentBackBufferIndex();
	return swapChainResources_[backBufferIndex_].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE SwapChain::GetCurrentBackBufferRTVHandle() {
	backBufferIndex_ = swapChain_->GetCurrentBackBufferIndex();
	return rtvMgr_.GetCPUDescriptorHandle(rtvIndex_[backBufferIndex_]);
}

bool SwapChain::CreateSwapChain() {
	// スワップチェーンの設定
	swapChainDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM;					// 色形式
	swapChainDesc_.SampleDesc.Count = 1;								// マルチサンプルしない
	swapChainDesc_.BufferCount = 2;										// ダブルバッファ
	swapChainDesc_.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;			// モニタにうつしたら、中身を破棄
	// コマンドキュー、ウィンドウハンドル、設定を渡して生成する
	HRESULT hr = device_.GetFactory()->CreateSwapChainForHwnd(
		cmd_.GetQueue(),
		static_cast<HWND>(windowHandle_),
		&swapChainDesc_,
		nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain_.GetAddressOf()));

	return SUCCEEDED(hr);
}

bool SwapChain::CreateResources() {
	// スワップチェーンからリソースを引っ張ってくる
	HRESULT hr = swapChain_->GetBuffer(0, IID_PPV_ARGS(&swapChainResources_[0]));
	// うまく取得出来なければ起動できない
	if (FAILED(hr)) {
		return false;
	}
	hr = swapChain_->GetBuffer(1, IID_PPV_ARGS(&swapChainResources_[1]));
	if (FAILED(hr)) {
		return false;
	}

	return true;
}

void SwapChain::CreateRTV() {
	// 1つめ
	if (!isRtvAllocated_) {
		rtvIndex_[0] = rtvMgr_.Allocate();
		rtvIndex_[1] = rtvMgr_.Allocate();
		isRtvAllocated_ = true;
	}
	rtvMgr_.CreateRTVTexture2d(rtvIndex_[0], swapChainResources_[0].Get(), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	// 2つめ
	rtvMgr_.CreateRTVTexture2d(rtvIndex_[1], swapChainResources_[1].Get(), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
}
