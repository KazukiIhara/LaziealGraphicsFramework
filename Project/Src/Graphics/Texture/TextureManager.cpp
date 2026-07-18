#include "TextureManager.h"

#include <algorithm>
#include <cctype>
#include <cstring>

#include "Core/Logger/Logger.h"
#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/DirectX/ViewManagers/SRVUAVManager/SRVUAVManager.h"
#include "Graphics/Texture/TextureInternal.h"

using namespace LGF;

namespace {
	TextureManager* g_textureManager = nullptr;
}

TextureManager::TextureManager(DirectXCore& directX) : directX_(directX) {
}

TextureManager::~TextureManager() {
	Finalize();
}

bool TextureManager::Initialize() {
	srvUavManager_ = directX_.GetResMgr().srvuav;
	if (!srvUavManager_ || !InitializeUploadContext()) {
		return false;
	}

	defaultTexture_ = CreateDefaultTexture();
	return IsValid(defaultTexture_);
}

void TextureManager::Finalize() {
	if (uploadFenceEvent_) {
		CloseHandle(uploadFenceEvent_);
		uploadFenceEvent_ = nullptr;
	}
	uploadFence_.Reset();
	uploadCommandList_.Reset();
	uploadAllocator_.Reset();
	defaultTexture_ = {};
	cache_.clear();
	entries_.clear();
	srvUavManager_ = nullptr;
}

Texture TextureManager::Load(const std::string& filePath, const TextureLoadDesc& desc) {
	if (filePath.empty()) {
		return defaultTexture_;
	}

	const std::filesystem::path path = std::filesystem::path(Logger::ConvertString(filePath)).lexically_normal();
	const std::string key = MakeCacheKey(path, desc);
	if (const auto it = cache_.find(key); it != cache_.end()) {
		return MakeTexture(it->second);
	}

	Texture texture = LoadImpl(path, desc);
	if (!texture.IsValid()) {
		Logger::Log("Texture load failed: " + filePath);
		return defaultTexture_;
	}

	cache_.emplace(key, texture.handle_.index);
	return texture;
}

const Texture& TextureManager::GetDefaultTexture() const {
	return defaultTexture_;
}

bool TextureManager::IsValid(const Texture& texture) const {
	return FindEntry(texture) != nullptr;
}

uint32_t TextureManager::GetWidth(const Texture& texture) const {
	const Entry* entry = FindEntry(texture);
	return entry ? static_cast<uint32_t>(entry->metadata.width) : 0;
}

uint32_t TextureManager::GetHeight(const Texture& texture) const {
	const Entry* entry = FindEntry(texture);
	return entry ? static_cast<uint32_t>(entry->metadata.height) : 0;
}

bool TextureManager::IsCubeMap(const Texture& texture) const {
	const Entry* entry = FindEntry(texture);
	return entry && entry->metadata.IsCubemap();
}

uint32_t TextureManager::GetSrvIndex(const Texture& texture) const {
	const Entry* entry = FindEntry(texture);
	if (!entry) {
		entry = FindEntry(defaultTexture_);
	}
	return entry ? entry->srvIndex : 0;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(const Texture& texture) const {
	return srvUavManager_->GetGPUDescriptorHandle(GetSrvIndex(texture));
}

bool TextureManager::InitializeUploadContext() {
	ID3D12Device* device = directX_.GetDevice();
	if (!device) {
		return false;
	}

	HRESULT hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&uploadAllocator_));
	if (FAILED(hr)) return false;
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, uploadAllocator_.Get(), nullptr, IID_PPV_ARGS(&uploadCommandList_));
	if (FAILED(hr)) return false;
	hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&uploadFence_));
	if (FAILED(hr)) return false;
	uploadFenceEvent_ = CreateEvent(nullptr, false, false, nullptr);
	return uploadFenceEvent_ != nullptr;
}

Texture TextureManager::CreateDefaultTexture() {
	DirectX::ScratchImage image;
	if (FAILED(image.Initialize2D(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, 2, 2, 1, 1))) {
		return {};
	}

	constexpr uint32_t pixels[4] = {
		0xFFFF00FFu, 0xFF202020u,
		0xFF202020u, 0xFFFF00FFu,
	};
	std::memcpy(image.GetPixels(), pixels, sizeof(pixels));

	Entry entry{};
	if (!CreateTexture(image, entry)) {
		return {};
	}
	entries_.push_back(std::move(entry));
	return MakeTexture(static_cast<uint32_t>(entries_.size() - 1));
}

Texture TextureManager::LoadImpl(const std::filesystem::path& filePath, const TextureLoadDesc& desc) {
	DirectX::ScratchImage image;
	if (!LoadTextureImage(filePath, desc, image)) {
		return {};
	}

	Entry entry{};
	if (!CreateTexture(image, entry)) {
		return {};
	}
	entries_.push_back(std::move(entry));
	return MakeTexture(static_cast<uint32_t>(entries_.size() - 1));
}

bool TextureManager::LoadTextureImage(const std::filesystem::path& filePath, const TextureLoadDesc& desc, DirectX::ScratchImage& result) const {
	DirectX::ScratchImage source;
	DirectX::TexMetadata metadata{};
	const std::wstring extension = filePath.extension().wstring();
	HRESULT hr = E_FAIL;

	if (_wcsicmp(extension.c_str(), L".dds") == 0) {
		hr = DirectX::LoadFromDDSFile(filePath.c_str(), DirectX::DDS_FLAGS_NONE, &metadata, source);
	} else {
		const auto flags = desc.colorSpace == TextureColorSpace::SRGB
			? DirectX::WIC_FLAGS_FORCE_SRGB
			: DirectX::WIC_FLAGS_IGNORE_SRGB;
		hr = DirectX::LoadFromWICFile(filePath.c_str(), flags, &metadata, source);
	}
	if (FAILED(hr)) return false;

	if (!desc.generateMipMaps || metadata.mipLevels > 1 || DirectX::IsCompressed(metadata.format)) {
		result = std::move(source);
		return true;
	}

	const DirectX::TEX_FILTER_FLAGS filter = desc.colorSpace == TextureColorSpace::SRGB
		? static_cast<DirectX::TEX_FILTER_FLAGS>(DirectX::TEX_FILTER_DEFAULT | DirectX::TEX_FILTER_SRGB)
		: DirectX::TEX_FILTER_DEFAULT;
	return SUCCEEDED(DirectX::GenerateMipMaps(
		source.GetImages(), source.GetImageCount(), source.GetMetadata(), filter, 0, result));
}

bool TextureManager::CreateTexture(const DirectX::ScratchImage& image, Entry& entry) {
	entry.metadata = image.GetMetadata();
	entry.resource = CreateTextureResource(entry.metadata);
	if (!entry.resource || !UploadTexture(entry.resource.Get(), image)) {
		return false;
	}

	entry.srvIndex = srvUavManager_->Allocate();
	if (entry.metadata.IsCubemap()) {
		srvUavManager_->CreateSrvTextureCubeMap(entry.srvIndex, entry.resource.Get(), entry.metadata.format);
	} else {
		srvUavManager_->CreateSrvTexture2d(entry.srvIndex, entry.resource.Get(), entry.metadata.format, static_cast<UINT>(entry.metadata.mipLevels));
	}
	return true;
}

ComPtr<ID3D12Resource> TextureManager::CreateTextureResource(const DirectX::TexMetadata& metadata) const {
	D3D12_RESOURCE_DESC desc{};
	desc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension);
	desc.Width = metadata.width;
	desc.Height = static_cast<UINT>(metadata.height);
	desc.DepthOrArraySize = static_cast<UINT16>(metadata.arraySize);
	desc.MipLevels = static_cast<UINT16>(metadata.mipLevels);
	desc.Format = metadata.format;
	desc.SampleDesc.Count = 1;

	D3D12_HEAP_PROPERTIES heap{};
	heap.Type = D3D12_HEAP_TYPE_DEFAULT;
	ComPtr<ID3D12Resource> resource;
	const HRESULT hr = directX_.GetDevice()->CreateCommittedResource(
		&heap, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr, IID_PPV_ARGS(&resource));
	return SUCCEEDED(hr) ? resource : nullptr;
}

bool TextureManager::UploadTexture(ID3D12Resource* texture, const DirectX::ScratchImage& image) {
	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	if (FAILED(DirectX::PrepareUpload(directX_.GetDevice(), image.GetImages(), image.GetImageCount(), image.GetMetadata(), subresources))) {
		return false;
	}

	const UINT count = static_cast<UINT>(subresources.size());
	const D3D12_RESOURCE_DESC textureDesc = texture->GetDesc();
	std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> layouts(count);
	std::vector<UINT> rowCounts(count);
	std::vector<UINT64> rowSizes(count);
	UINT64 uploadSize = 0;
	directX_.GetDevice()->GetCopyableFootprints(&textureDesc, 0, count, 0, layouts.data(), rowCounts.data(), rowSizes.data(), &uploadSize);

	D3D12_HEAP_PROPERTIES heap{};
	heap.Type = D3D12_HEAP_TYPE_UPLOAD;
	D3D12_RESOURCE_DESC buffer{};
	buffer.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	buffer.Width = uploadSize;
	buffer.Height = 1;
	buffer.DepthOrArraySize = 1;
	buffer.MipLevels = 1;
	buffer.SampleDesc.Count = 1;
	buffer.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	ComPtr<ID3D12Resource> upload;
	if (FAILED(directX_.GetDevice()->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &buffer,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&upload)))) return false;

	uint8_t* mapped = nullptr;
	if (FAILED(upload->Map(0, nullptr, reinterpret_cast<void**>(&mapped)))) return false;
	for (UINT i = 0; i < count; ++i) {
		uint8_t* dst = mapped + layouts[i].Offset;
		const uint8_t* src = static_cast<const uint8_t*>(subresources[i].pData);
		for (UINT row = 0; row < rowCounts[i]; ++row) {
			std::memcpy(dst + static_cast<size_t>(row) * layouts[i].Footprint.RowPitch,
				src + static_cast<size_t>(row) * subresources[i].RowPitch,
				static_cast<size_t>(rowSizes[i]));
		}
	}
	upload->Unmap(0, nullptr);

	for (UINT i = 0; i < count; ++i) {
		D3D12_TEXTURE_COPY_LOCATION dst{};
		dst.pResource = texture;
		dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dst.SubresourceIndex = i;
		D3D12_TEXTURE_COPY_LOCATION src{};
		src.pResource = upload.Get();
		src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		src.PlacedFootprint = layouts[i];
		uploadCommandList_->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
	}

	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = texture;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	uploadCommandList_->ResourceBarrier(1, &barrier);
	return ExecuteUpload();
}

bool TextureManager::ExecuteUpload() {
	if (FAILED(uploadCommandList_->Close())) return false;
	ID3D12CommandList* lists[] = { uploadCommandList_.Get() };
	directX_.GetCommandQueue()->ExecuteCommandLists(1, lists);
	const uint64_t value = ++uploadFenceValue_;
	if (FAILED(directX_.GetCommandQueue()->Signal(uploadFence_.Get(), value))) return false;
	if (uploadFence_->GetCompletedValue() < value) {
		if (FAILED(uploadFence_->SetEventOnCompletion(value, uploadFenceEvent_))) return false;
		WaitForSingleObject(uploadFenceEvent_, INFINITE);
	}
	if (FAILED(uploadAllocator_->Reset())) return false;
	return SUCCEEDED(uploadCommandList_->Reset(uploadAllocator_.Get(), nullptr));
}

std::string TextureManager::MakeCacheKey(const std::filesystem::path& filePath, const TextureLoadDesc& desc) const {
	std::string key = Logger::ConvertString(filePath.wstring());
	std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	key += desc.colorSpace == TextureColorSpace::SRGB ? "|srgb" : "|linear";
	key += desc.generateMipMaps ? "|mips" : "|nomips";
	return key;
}

const TextureManager::Entry* TextureManager::FindEntry(const Texture& texture) const {
	if (texture.handle_.generation == 0 || texture.handle_.index >= entries_.size()) return nullptr;
	const Entry& entry = entries_[texture.handle_.index];
	return entry.generation == texture.handle_.generation ? &entry : nullptr;
}

Texture TextureManager::MakeTexture(uint32_t index) const {
	if (index >= entries_.size()) return {};
	return Texture(Texture::Handle{ index, entries_[index].generation });
}

void LGF::TextureInternal::SetManager(TextureManager* manager) {
	g_textureManager = manager;
}

void LGF::TextureInternal::ResetManager() {
	g_textureManager = nullptr;
}

Texture LGF::TextureInternal::Load(const std::string& filePath, const TextureLoadDesc& desc) {
	return g_textureManager ? g_textureManager->Load(filePath, desc) : Texture{};
}

bool LGF::TextureInternal::IsValid(const Texture& texture) {
	return g_textureManager && g_textureManager->IsValid(texture);
}

uint32_t LGF::TextureInternal::GetWidth(const Texture& texture) {
	return g_textureManager ? g_textureManager->GetWidth(texture) : 0;
}

uint32_t LGF::TextureInternal::GetHeight(const Texture& texture) {
	return g_textureManager ? g_textureManager->GetHeight(texture) : 0;
}

bool LGF::TextureInternal::IsCubeMap(const Texture& texture) {
	return g_textureManager && g_textureManager->IsCubeMap(texture);
}
