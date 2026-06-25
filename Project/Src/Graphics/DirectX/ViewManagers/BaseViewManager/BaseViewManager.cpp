#include "BaseViewManager.h"

#include <cassert>

#include "Graphics/DirectX/DescriptorHeap/DescriptorHeapFactory.h"
#include "Graphics/DirectX/Device/Device.h"

using namespace LGF;

BaseViewManager::BaseViewManager(Device& device, DescriptorHeapFactory& descriptorHeapManager)
	: device_(device), descriptorHeapManager_(descriptorHeapManager) {
}

void BaseViewManager::Initialize(
	D3D12_DESCRIPTOR_HEAP_TYPE heapType,
	uint32_t maxViewCount,
	bool shaderVisible) {

	maxViewCount_ = maxViewCount;
	isShaderVisible_ = shaderVisible;
	useIndex_ = 0u;

	descriptorHeap_ = descriptorHeapManager_.CreateDescriptorHeap(
		heapType,
		maxViewCount_,
		isShaderVisible_);

	descriptorSize_ = device_.GetDevice()->GetDescriptorHandleIncrementSize(heapType);
}

uint32_t BaseViewManager::Allocate() {
	assert(useIndex_ < maxViewCount_);

	uint32_t index = useIndex_;
	++useIndex_;
	return index;
}

D3D12_CPU_DESCRIPTOR_HANDLE BaseViewManager::GetCPUDescriptorHandle(uint32_t index)const {
	assert(index < maxViewCount_);

	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap_->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += static_cast<SIZE_T>(descriptorSize_) * index;
	return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE BaseViewManager::GetGPUDescriptorHandle(uint32_t index)const {
	assert(isShaderVisible_);
	assert(index < maxViewCount_);

	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap_->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += static_cast<UINT64>(descriptorSize_) * index;
	return handleGPU;
}

ID3D12DescriptorHeap* BaseViewManager::GetDescriptorHeap()const {
	return descriptorHeap_.Get();
}

uint32_t BaseViewManager::GetDescriptorSize()const {
	return descriptorSize_;
}

uint32_t BaseViewManager::GetUseIndex()const {
	return useIndex_;
}

bool BaseViewManager::IsLowerViewMax()const {
	return useIndex_ < maxViewCount_;
}
