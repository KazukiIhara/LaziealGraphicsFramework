#include "DirectXCore.h"

#include "Window/Window.h"
#include "Device/Device.h"
#include "Command/Command.h"
#include "DescriptorHeap/DescriptorHeapFactory.h"
#include "Viewmanagers/RTVManager/RTVManager.h"
#include "Viewmanagers/DSVManager/DSVManager.h"
#include "Viewmanagers/SRVUAVManager/SRVUAVManager.h"
#include "ResourceFactory/ResourceFactory.h"
#include "Viewport/Viewport.h"
#include "ScissorRect/ScissorRect.h"
#include "SwapChain/SwapChain.h"
#include "DepthStencil/DepthStencil.h"
#include "Fence/Fence.h"
#include "ShaderCompiler/ShaderCompiler.h"

using namespace LGF;

DirectXCore::DirectXCore() {
}

DirectXCore::~DirectXCore() = default;

bool DirectXCore::Initialize(Window& window) {
	renderWidth_ = window.GetWndSetting().wndSize.width;
	renderHeight_ = window.GetWndSetting().wndSize.height;

	device_ = std::make_unique<Device>();
	command_ = std::make_unique<Command>(*device_);
	descriptorHeapFactory_ = std::make_unique<DescriptorHeapFactory>(*device_);
	rtvManager_ = std::make_unique<RTVManager>(*device_, *descriptorHeapFactory_);
	dsvManager_ = std::make_unique<DSVManager>(*device_, *descriptorHeapFactory_);
	srvUavManager_ = std::make_unique<SRVUAVManager>(*device_, *descriptorHeapFactory_);
	resourceFactory_ = std::make_unique<ResourceFactory>(*device_);
	viewport_ = std::make_unique<Viewport>(*command_);
	scissorRect_ = std::make_unique<ScissorRect>(*command_);
	swapChain_ = std::make_unique<SwapChain>(window, *device_, *command_, *rtvManager_, *viewport_, *scissorRect_);
	depthStencil_ = std::make_unique<DepthStencil>(*resourceFactory_, *dsvManager_);
	fence_ = std::make_unique<Fence>(*device_, *command_);
	shaderCompiler_ = std::make_unique<ShaderCompiler>();

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
	depthStencil_->Initialize(renderWidth_, renderHeight_);
	if (!fence_->Initialize()) {
		return false;
	}
	if (!shaderCompiler_->Initialize()) {
		return false;
	}

	return true;
}

void DirectXCore::Finalize() {
	shaderCompiler_.reset();
	fence_.reset();
	depthStencil_.reset();
	swapChain_.reset();
	scissorRect_.reset();
	viewport_.reset();
	resourceFactory_.reset();
	srvUavManager_.reset();
	dsvManager_.reset();
	rtvManager_.reset();
	descriptorHeapFactory_.reset();
	command_.reset();
	device_.reset();
}

void DirectXCore::BeginFrame() {
	command_->BeginFrame();
	swapChain_->BeginFrame();

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = swapChain_->GetCurrentBackBufferRTVHandle();
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = depthStencil_->GetDSVHandleCPU();
	command_->GetList()->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);

	constexpr float clearColor[] = { 0.1f, 0.25f, 0.5f, 1.0f };
	command_->GetList()->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	command_->GetList()->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	viewport_->BeginFrame(renderWidth_, renderHeight_);
	scissorRect_->BeginFrame(renderWidth_, renderHeight_);
}

void DirectXCore::EndFrame() {
	swapChain_->EndFrame();
	command_->EndFrame();
	swapChain_->Present();
	fence_->EndFrame();
}

ID3D12Device* DirectXCore::GetDevice() {
	return device_->GetDevice();
}

ID3D12GraphicsCommandList* DirectXCore::GetCommandList() {
	return command_->GetList();
}

ID3D12GraphicsCommandList6* DirectXCore::GetCommandList6() {
	return command_->GetList6();
}

DirectXCore::ResMgr DirectXCore::GetResMgr() {
	return ResMgr(rtvManager_.get(), dsvManager_.get(), srvUavManager_.get());
}

ResourceFactory* DirectXCore::GetResourceFactory() {
	return resourceFactory_.get();
}

ShaderCompiler* DirectXCore::GetShaderCompiler() {
	return shaderCompiler_.get();
}
