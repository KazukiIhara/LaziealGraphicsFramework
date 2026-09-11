#include "TextureManager.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <stdexcept>
#include <format>

#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/DirectX/ResourceFactory/ResourceFactory.h"
#include "Graphics/DirectX/ViewManagers/SRVUAVManager/SRVUAVManager.h"
#include "DirectXTex/Common/d3dx12.h"

using namespace LGF;

TextureManager::TextureManager(DirectXCore& directX) : directX_(directX) {}
TextureManager::~TextureManager() = default;
bool TextureManager::Initialize() { return true; }
void TextureManager::Finalize() { textures_.clear(); }

const TextureData& TextureManager::Load(const std::string& filePath, const TextureLoadDesc& desc) {
    const auto check = [&filePath](HRESULT hr, const char* operation) {
        if (FAILED(hr)) {
            throw std::runtime_error(std::format("Texture '{}': {} failed (0x{:08X}).",
                filePath, operation, static_cast<uint32_t>(hr)));
        }
    };
    if (filePath.empty()) {
        throw std::invalid_argument("Texture path must not be empty.");
    }
    const auto path = std::filesystem::absolute(std::filesystem::path(
        std::u8string(filePath.begin(), filePath.end()))).lexically_normal();
    const auto key = std::format(L"{}|{}|{}|{}", path.native(),
        static_cast<int>(desc.dimension), static_cast<int>(desc.colorSpace), desc.generateMipMaps);
    if (const auto it = textures_.find(key); it != textures_.end()) {
        return it->second;
    }

    DirectX::ScratchImage image;
    auto extension = path.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (extension == ".dds") {
        check(DirectX::LoadFromDDSFile(path.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, image), "LoadDDS");
    } else if (extension == ".tga") {
        check(DirectX::LoadFromTGAFile(path.c_str(), DirectX::TGA_FLAGS_NONE, nullptr, image), "LoadTGA");
    } else if (extension == ".hdr") {
        check(DirectX::LoadFromHDRFile(path.c_str(), nullptr, image), "LoadHDR");
    } else {
        check(DirectX::LoadFromWICFile(path.c_str(), DirectX::WIC_FLAGS_FORCE_RGB, nullptr, image), "LoadWIC");
    }

    const auto& metadata = image.GetMetadata();
    const bool cube = metadata.IsCubemap();
    if (metadata.dimension != DirectX::TEX_DIMENSION_TEXTURE2D ||
        metadata.arraySize != (cube ? 6u : 1u) ||
        (desc.dimension == TextureDimension::Texture2D && cube) ||
        (desc.dimension == TextureDimension::TextureCube && !cube)) {
        throw std::runtime_error("Texture '" + filePath + "': expected a single Texture2D or cubemap.");
    }
    const DXGI_FORMAT format = desc.colorSpace == TextureColorSpace::SRGB
        ? DirectX::MakeSRGB(metadata.format) : DirectX::MakeLinear(metadata.format);
    if (!image.OverrideFormat(format)) {
        throw std::runtime_error("Texture '" + filePath + "': unsupported color-space format.");
    }

    // DDS の既存ミップは保持。圧縮画像にミップがない場合は展開して生成する。
    if (desc.generateMipMaps && image.GetMetadata().mipLevels == 1 &&
        (metadata.width > 1 || metadata.height > 1)) {
        if (DirectX::IsCompressed(image.GetMetadata().format)) {
            DirectX::ScratchImage decompressed;
            check(DirectX::Decompress(image.GetImages(), image.GetImageCount(), image.GetMetadata(),
                DXGI_FORMAT_UNKNOWN, decompressed), "Decompress");
            image = std::move(decompressed);
        }
        DirectX::ScratchImage mipImage;
        check(DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(),
            DirectX::TEX_FILTER_DEFAULT, 0, mipImage), "GenerateMipMaps");
        image = std::move(mipImage);
    }

    auto* views = directX_.GetResMgr().srvuav;
    if (!views->IsLowerViewMax()) {
        throw std::runtime_error("Texture descriptor heap is full.");
    }
    TextureData texture{};
    texture.metaData = image.GetMetadata();
    auto* device = directX_.GetDevice();
    D3D12_FEATURE_DATA_FORMAT_SUPPORT support{ texture.metaData.format };
    check(device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &support, sizeof(support)), "CheckFormatSupport");
    if (!(support.Support1 & D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE)) {
        throw std::runtime_error("Texture '" + filePath + "': format does not support filtered sampling.");
    }
    check(DirectX::CreateTexture(device, texture.metaData, &texture.resource), "CreateTexture");

    std::vector<D3D12_SUBRESOURCE_DATA> subresources;
    check(DirectX::PrepareUpload(device, image.GetImages(), image.GetImageCount(),
        texture.metaData, subresources), "PrepareUpload");
    const auto count = static_cast<UINT>(subresources.size());
    const auto uploadSize = GetRequiredIntermediateSize(texture.resource.Get(), 0, count);
    auto upload = directX_.GetResourceFactory()->CreateUploadBufferResource(uploadSize);
    if (!upload) {
        throw std::runtime_error("Texture upload buffer allocation failed.");
    }

    // 描画中のリストを変更しない専用リスト。初回ロードだけ同期転送する。
    ComPtr<ID3D12CommandAllocator> allocator;
    ComPtr<ID3D12GraphicsCommandList> list;
    ComPtr<ID3D12Fence> fence;
    check(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator)), "CreateAllocator");
    check(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator.Get(),
        nullptr, IID_PPV_ARGS(&list)), "CreateCommandList");
    check(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)), "CreateFence");
    if (UpdateSubresources(list.Get(), texture.resource.Get(), upload.Get(), 0, 0, count, subresources.data()) == 0) {
        throw std::runtime_error("Texture upload failed.");
    }
    const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(texture.resource.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    list->ResourceBarrier(1, &barrier);
    check(list->Close(), "CloseCommandList");
    ID3D12CommandList* lists[] = { list.Get() };
    directX_.GetCommandQueue()->ExecuteCommandLists(1, lists);
    check(directX_.GetCommandQueue()->Signal(fence.Get(), 1), "Signal");
    // nullptr イベントは完了までブロックする。転送用リソースは待機後に解放。
    check(fence->SetEventOnCompletion(1, nullptr), "WaitForUpload");

    texture.srvIndex = views->Allocate();
    if (cube) {
        views->CreateSrvTextureCubeMap(texture.srvIndex, texture.resource.Get(), texture.metaData.format);
    } else {
        views->CreateSrvTexture2d(texture.srvIndex, texture.resource.Get(), texture.metaData.format,
            static_cast<UINT>(texture.metaData.mipLevels));
    }
    return textures_.emplace(key, std::move(texture)).first->second;
}
