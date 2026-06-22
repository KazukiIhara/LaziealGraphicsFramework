#include "SRVUAVManager.h"

#include <cassert>

#include "DescriptorHeap/DescriptorHeapFactory.h"
#include "Device/Device.h"

using namespace LGF;

SRVUAVManager::SRVUAVManager(Device& device, DescriptorHeapFactory& descriptorHeapFactory)
	: BaseViewManager(device, descriptorHeapFactory), device_(device) {
}

void SRVUAVManager::Initialize(uint32_t maxViewCount) {
	BaseViewManager::Initialize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		maxViewCount,
		true);
}

void SRVUAVManager::CreateSrvTexture2d(uint32_t viewIndex, ID3D12Resource* resource, DXGI_FORMAT format, UINT mipLevels) {
	assert(resource);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = mipLevels;

	device_.GetDevice()->CreateShaderResourceView(
		resource,
		&srvDesc,
		GetCPUDescriptorHandle(viewIndex));
}

void SRVUAVManager::CreateSrvTextureCubeMap(uint32_t viewIndex, ID3D12Resource* resource, DXGI_FORMAT format) {
	assert(resource);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = UINT_MAX;
	srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;

	device_.GetDevice()->CreateShaderResourceView(
		resource,
		&srvDesc,
		GetCPUDescriptorHandle(viewIndex));
}

void SRVUAVManager::CreateSrvStructuredBuffer(uint32_t viewIndex, ID3D12Resource* resource, uint32_t numElements, UINT structureByteStride) {
	assert(resource);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	srvDesc.Buffer.NumElements = numElements;
	srvDesc.Buffer.StructureByteStride = structureByteStride;

	device_.GetDevice()->CreateShaderResourceView(
		resource,
		&srvDesc,
		GetCPUDescriptorHandle(viewIndex));
}

void SRVUAVManager::CreateSrvByteAddressBuffer(uint32_t viewIndex, ID3D12Resource* resource, uint32_t byteSize) {
	assert(resource);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = byteSize / 4;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
	srvDesc.Buffer.StructureByteStride = 0;

	device_.GetDevice()->CreateShaderResourceView(
		resource,
		&srvDesc,
		GetCPUDescriptorHandle(viewIndex));
}

void SRVUAVManager::CreateUavStructuredBuffer(uint32_t viewIndex, ID3D12Resource* resource, uint32_t numElements, UINT structureByteStride) {
	assert(resource);

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = numElements;
	uavDesc.Buffer.CounterOffsetInBytes = 0;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
	uavDesc.Buffer.StructureByteStride = structureByteStride;

	device_.GetDevice()->CreateUnorderedAccessView(
		resource,
		nullptr,
		&uavDesc,
		GetCPUDescriptorHandle(viewIndex));
}

void SRVUAVManager::CreateConstantBufferView(uint32_t viewIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation, uint32_t sizeInBytes) {
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc{};
	cbvDesc.BufferLocation = bufferLocation;
	cbvDesc.SizeInBytes = (sizeInBytes + 255u) & ~255u;

	device_.GetDevice()->CreateConstantBufferView(
		&cbvDesc,
		GetCPUDescriptorHandle(viewIndex));
}
