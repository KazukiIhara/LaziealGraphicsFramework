#include "BaseViewManager.h"

#include <cassert>
#include <algorithm>

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
	freeIndices_.clear();

	descriptorHeap_ = descriptorHeapManager_.CreateDescriptorHeap(
		heapType,
		maxViewCount_,
		isShaderVisible_);

	descriptorSize_ = device_.GetDevice()->GetDescriptorHandleIncrementSize(heapType);
}

uint32_t BaseViewManager::Allocate() {
	if (!freeIndices_.empty()) {
		const uint32_t index = freeIndices_.back();
		freeIndices_.pop_back();
		return index;
	}
	assert(useIndex_ < maxViewCount_);

	uint32_t index = useIndex_;
	++useIndex_;
	return index;
}

void BaseViewManager::Release(uint32_t index) {
	assert(index < useIndex_);
	assert(std::find(freeIndices_.begin(), freeIndices_.end(), index) == freeIndices_.end());
	freeIndices_.push_back(index);
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
	return !freeIndices_.empty() || useIndex_ < maxViewCount_;
}

bool BaseViewManager::CanAllocate(uint32_t count) const {
	const uint32_t unusedCount = maxViewCount_ - useIndex_;
	return count <= unusedCount + static_cast<uint32_t>(freeIndices_.size());
}
