#include "SwapChain.h"

#include <cassert>

#include "Window/Window.h"
#include "Device/Device.h"
#include "Command/Command.h"
#include "ViewManagers/RTVManager/RTVManager.h"
#include "Viewport/Viewport.h"
#include "ScissorRect/ScissorRect.h"

using namespace LGF;

SwapChain::SwapChain(Window& win, Device& device, Command& cmd,
	RTVManager& rtvMgr, Viewport& viewport, ScissorRect& scissorRect)
	:win_(win), device_(device), cmd_(cmd), rtvMgr_(rtvMgr), viewport_(viewport), scissorRect_(scissorRect) {
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
	return rtvMgr_.GetDescriptorHandleCPU(rtvIndex_[backBufferIndex_]);
}

bool SwapChain::CreateSwapChain() {
	// スワップチェーンの設定
	swapChainDesc_.Width = win_.GetWndSetting().wndSize.width;			// 画面の幅、ウィンドウのクライアント領域を同じものにしておく
	swapChainDesc_.Height = win_.GetWndSetting().wndSize.height;		// 画面の高さ、ウィンドウのクライアント領域を同じものしておく
	swapChainDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM;					// 色形式
	swapChainDesc_.SampleDesc.Count = 1;								// マルチサンプルしない
	swapChainDesc_.BufferCount = 2;										// ダブルバッファ
	swapChainDesc_.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;			// モニタにうつしたら、中身を破棄
	// コマンドキュー、ウィンドウハンドル、設定を渡して生成する
	HRESULT hr = device_.GetFactory()->CreateSwapChainForHwnd(cmd_.GetQueue(), win_.GetHwnd(), &swapChainDesc_,
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
	rtvIndex_[0] = rtvMgr_.Allocate();
	rtvMgr_.CreateRTVTexture2d(rtvIndex_[0], swapChainResources_[0].Get(), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	// 2つめ
	rtvIndex_[1] = rtvMgr_.Allocate();
	rtvMgr_.CreateRTVTexture2d(rtvIndex_[1], swapChainResources_[1].Get(), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
}
