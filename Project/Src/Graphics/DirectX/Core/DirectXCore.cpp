#include "DirectXCore.h"

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

	return false;
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
