#include "ResourceFactory.h"

#include <cassert>

#include "Graphics/DirectX/Device/Device.h"

using namespace LGF;

ResourceFactory::ResourceFactory(Device& device)
	: device_(device) {
}

ComPtr<ID3D12Resource> ResourceFactory::CreateUploadBufferResource(size_t sizeInBytes) {
	assert(sizeInBytes > 0);
	// UploadHeap設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	// バッファ設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = sizeInBytes;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// リソースを生成
	ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device_.GetDevice()->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));

	return resource;
}

ComPtr<ID3D12Resource> ResourceFactory::CreateUAVBufferResource(size_t sizeInBytes) {
	// DefaultHeap設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	// バッファ設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = sizeInBytes;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	// リソースを生成
	ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device_.GetDevice()->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));

	return resource;
}

ComPtr<ID3D12Resource> ResourceFactory::CreateUAVTextureResource(
	uint32_t width,
	uint32_t height,
	uint16_t arraySize,
	uint16_t mipLevels,
	DXGI_FORMAT format) {

	assert(width > 0u);
	assert(height > 0u);
	assert(arraySize > 0u);
	assert(mipLevels > 0u);
	assert(format != DXGI_FORMAT_UNKNOWN);

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Width = width;
	resourceDesc.Height = height;
	resourceDesc.DepthOrArraySize = arraySize;
	resourceDesc.MipLevels = mipLevels;
	resourceDesc.Format = format;
	resourceDesc.SampleDesc.Count = 1u;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	ComPtr<ID3D12Resource> resource;
	const HRESULT result = device_.GetDevice()->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		nullptr,
		IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(result));
	return resource;
}

ComPtr<ID3D12Resource> ResourceFactory::CreateRenderTargetTextureResource(
	int32_t width,
	int32_t height,
	DXGI_FORMAT resourceFormat,
	DXGI_FORMAT clearFormat,
	const Vector4& clearColor) {

	assert(width > 0);
	assert(height > 0);

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = static_cast<UINT64>(width);
	resourceDesc.Height = static_cast<UINT>(height);
	resourceDesc.MipLevels = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Format = resourceFormat;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	// ClearValueのFormatはリソース本体ではなくRTV側のFormatに合わせる。
	// ここを分けておくと、後でHDRやTypelessなRenderTextureを追加しやすい。
	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = clearFormat;
	clearValue.Color[0] = clearColor.x;
	clearValue.Color[1] = clearColor.y;
	clearValue.Color[2] = clearColor.z;
	clearValue.Color[3] = clearColor.w;

	ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device_.GetDevice()->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		&clearValue,
		IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));

	return resource;
}

ComPtr<ID3D12Resource> ResourceFactory::CreateDepthStencilTextureResource(
	int32_t width,
	int32_t height,
	DXGI_FORMAT format) {

	// リソース設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width;
	resourceDesc.Height = height;
	resourceDesc.MipLevels = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Format = format;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	// Heap設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	// クリア値設定
	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = format;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;

	// リソースを生成
	ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device_.GetDevice()->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&clearValue,
		IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));

	return resource;
}

ComPtr<ID3D12Resource> ResourceFactory::CreateDepthStencilTextureResource(
	int32_t width,
	int32_t height,
	DXGI_FORMAT format,
	DXGI_FORMAT clearFormat) {

	// リソース設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width;
	resourceDesc.Height = height;
	resourceDesc.MipLevels = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Format = format;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	// Heap設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	// クリア値設定
	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = clearFormat;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;

	// リソースを生成
	ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device_.GetDevice()->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&clearValue,
		IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));

	return resource;
}
