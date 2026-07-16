#pragma once

#include <cstdint>

#include <d3d12.h>
#include <dxgi1_6.h>

#include "Graphics/DirectX/ComPtr/ComPtr.h"
#include "LGF/Math/Vector4.h"

namespace LGF {

	class ResourceFactory;
	class RTVManager;
	class SRVUAVManager;

	struct RenderTextureDesc {
		uint32_t width = 0u;
		uint32_t height = 0u;
		DXGI_FORMAT resourceFormat = DXGI_FORMAT_R8G8B8A8_TYPELESS;
		DXGI_FORMAT rtvFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		DXGI_FORMAT srvFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		Vector4 clearColor = { 0.1f, 0.25f, 0.5f, 1.0f };
	};

	/// <summary>
	/// RenderTargetとShader入力の両方で使えるTextureリソース
	/// </summary>
	class RenderTexture {
	public:
		RenderTexture(
			ResourceFactory& resourceFactory,
			RTVManager& rtvManager,
			SRVUAVManager& srvUavManager);
		~RenderTexture();

		RenderTexture(const RenderTexture&) = delete;
		RenderTexture& operator=(const RenderTexture&) = delete;
		RenderTexture(RenderTexture&&) = delete;
		RenderTexture& operator=(RenderTexture&&) = delete;

		bool Initialize(const RenderTextureDesc& desc);
		bool Resize(uint32_t width, uint32_t height);

		void Transition(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES nextState);
		void Clear(ID3D12GraphicsCommandList* commandList);

		ID3D12Resource* GetResource() const;
		D3D12_CPU_DESCRIPTOR_HANDLE GetRTVHandleCPU() const;
		D3D12_GPU_DESCRIPTOR_HANDLE GetSRVHandleGPU() const;

		uint32_t GetWidth() const;
		uint32_t GetHeight() const;
		const Vector4& GetClearColor() const;
		D3D12_RESOURCE_STATES GetCurrentState() const;

	private:
		bool CreateResourceAndViews();

	private:
		ResourceFactory& resourceFactory_;
		RTVManager& rtvManager_;
		SRVUAVManager& srvUavManager_;

		RenderTextureDesc desc_{};
		ComPtr<ID3D12Resource> resource_ = nullptr;

		uint32_t rtvIndex_ = 0u;
		uint32_t srvIndex_ = 0u;
		bool isViewAllocated_ = false;

		// 現在のResourceStateをRenderTexture側で管理しておく。
		// 呼び出し側は「RTにしたい」「SRVにしたい」という意図だけ渡せばよくなる。
		D3D12_RESOURCE_STATES currentState_ = D3D12_RESOURCE_STATE_RENDER_TARGET;
	};
}
