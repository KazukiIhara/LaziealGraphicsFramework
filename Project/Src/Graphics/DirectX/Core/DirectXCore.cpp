#include "DirectXCore.h"

#include "Window/Window.h"
#include "Device/Device.h"
#include "Command/Command.h"
#include "DescriptorHeap/DescriptorHeapFactory.h"
#include "RTVManager/RTVManager.h"
#include "DSVManager/DSVManager.h"
#include "SRVUAVManager/SRVUAVManager.h"
#include "ResourceFactory/ResourceFactory.h"
#include "Viewport/Viewport.h"
#include "ScissorRect/ScissorRect.h"
#include "SwapChain/SwapChain.h"
#include "DepthStencil/DepthStencil.h"
#include "Fence/Fence.h"

using namespace LGF;

DirectXCore::DirectXCore() {
}

bool DirectXCore::Initialize(Window& window) {
	device_ = std::make_unique<Device>();
	command_ = std::make_unique<Command>(device_.get());
	descriptorHeapFactory_ = std::make_unique<DescriptorHeapFactory>(device_.get());
	rtvManager_ = std::make_unique<RTVManager>(device_.get(), descriptorHeapFactory_.get());
	dsvManager_ = std::make_unique<DSVManager>(device_.get(), descriptorHeapFactory_.get());
	srvUavManager_ = std::make_unique<SRVUAVManager>(device_.get(), descriptorHeapFactory_.get());
	resourceFactory_ = std::make_unique<ResourceFactory>(device_.get());
	viewport_ = std::make_unique<Viewport>(command_.get());
	scissorRect_ = std::make_unique<ScissorRect>(command_.get());
	swapChain_ = std::make_unique<SwapChain>(window, device_.get(), command_.get(), rtvManager_.get(), viewport_.get(), scissorRect_.get());
	depthStencil_ = std::make_unique<DepthStencil>(resourceFactory_.get(), dsvManager_.get());
	fence_ = std::make_unique<Fence>(device_.get(), command_.get());

	if (Device::InitializeResult::kSuccess != device_->Initialize()) {
		return false;
	}
	if (!command_->Initialize()) {
		return false;
	}
	rtvManager_->Initialize();
	dsvManager_->Initialize();
	srvUavManager_->Initialize();
	if (!swapChain_->Initialize()) {
		return false;
	}
	depthStencil_->Initialize(window.GetWndSetting().wndSize.width, window.GetWndSetting().wndSize.height);
	fence_->Initialize();

	return true;
}

void DirectXCore::Finalize() {
}

void DirectXCore::BeginFrame() {
}

void DirectXCore::EndFrame() {
}

ID3D12Device* DirectXCore::GetDevice() {
	return nullptr;
}

ID3D12GraphicsCommandList* DirectXCore::GetCommandList() {
	return nullptr;
}
