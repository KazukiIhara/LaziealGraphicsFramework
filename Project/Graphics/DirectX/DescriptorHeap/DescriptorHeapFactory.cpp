#include "DescriptorHeapFactory.h"

#include <cassert>

#include "Device/Device.h"

using namespace LGF;

DescriptorHeapFactory::DescriptorHeapFactory(Device& device)
	: device_(device) {
}

ComPtr<ID3D12DescriptorHeap> DescriptorHeapFactory::CreateDescriptorHeap(
	D3D12_DESCRIPTOR_HEAP_TYPE heapType,
	UINT numDescriptors,
	bool shaderVisible) {

	// DescriptorHeap設定
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heapType;
	descriptorHeapDesc.NumDescriptors = numDescriptors;

	// シェーダ可視設定
	if (shaderVisible) {
		descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	}
	else {
		descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	}

	// DescriptorHeapを生成
	ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
	HRESULT hr = device_.GetDevice()->CreateDescriptorHeap(
		&descriptorHeapDesc,
		IID_PPV_ARGS(&descriptorHeap));
	assert(SUCCEEDED(hr));

	return descriptorHeap;
}
