#pragma once

#include <cstdint>
#include <memory>
#include <span>

#include <d3d12.h>

#include "Graphics/DirectX/ComPtr/ComPtr.h"

namespace LGF {

	class Window;
	class Device;
	class Command;
	class DescriptorHeapFactory;
	class RTVManager;
	class DSVManager;
	class SRVUAVManager;
	class ResourceFactory;
	class Viewport;
	class ScissorRect;
	class SwapChain;
	class DepthStencil;
	class Fence;
	class ShaderCompiler;
	class PipelineManager;
	class RenderTexture;
	class RenderTextureManager;
	class DirectionalLightSystem;
	class Camera3DSystem;
	class SkyboxSystem;
	class ShadowMapSystem;

	/// <summary>
	/// DirectXコアシステム
	/// </summary>
	class DirectXCore {
	public:
		struct ResMgr {
			RTVManager* rtv;
			DSVManager* dsv;
			SRVUAVManager* srvuav;
		};
	public:
		DirectXCore(Window& window);
		~DirectXCore();

		bool Initialize();
		bool BeginFrame();
		void SetRenderTargets(std::span<RenderTexture* const> renderTextures, bool useDepthStencil = true);
		void ClearRenderTargets(std::span<RenderTexture* const> renderTextures);
		void RestoreGBufferRenderTargets();
		void DrawLightingToSceneColor(
			PipelineManager& pipelineManager,
			DirectionalLightSystem& lightSystem,
			Camera3DSystem& cameraSystem,
			SkyboxSystem& skyboxSystem,
			ShadowMapSystem& shadowMapSystem);
		void DrawSceneToSwapChain(PipelineManager& pipelineManager);
		void DrawDepthToSwapChain(PipelineManager& pipelineManager);
		void SetExposure(float exposureEV);
		void EndFrame();
		void Finalize();

		ID3D12Device* GetDevice();
		ID3D12GraphicsCommandList* GetCommandList();
		ID3D12GraphicsCommandList6* GetCommandList6();
		ID3D12CommandQueue* GetCommandQueue();

		ResourceFactory* GetResourceFactory();
		ShaderCompiler* GetShaderCompiler();
		RenderTextureManager* GetRenderTextureManager();

		ResMgr GetResMgr();

	private:
		struct ToneMappingForGPU {
			float exposureEV = 0.0f;
			float padding[3]{};
		};
		static_assert(sizeof(ToneMappingForGPU) == 16u);

		bool Resize(uint32_t width, uint32_t height);

	private:
		uint32_t renderWidth_ = 0u;
		uint32_t renderHeight_ = 0u;

		std::unique_ptr<Device> device_;
		std::unique_ptr<Command> command_;
		std::unique_ptr<DescriptorHeapFactory> descriptorHeapFactory_;
		std::unique_ptr<RTVManager> rtvManager_;
		std::unique_ptr<DSVManager> dsvManager_;
		std::unique_ptr<SRVUAVManager> srvUavManager_;
		std::unique_ptr<ResourceFactory> resourceFactory_;
		std::unique_ptr<Viewport> viewport_;
		std::unique_ptr<ScissorRect> scissorRect_;
		std::unique_ptr<SwapChain> swapChain_;
		std::unique_ptr<DepthStencil> depthStencil_;
		std::unique_ptr<Fence> fence_;
		std::unique_ptr<ShaderCompiler> shaderCompiler_;
		std::unique_ptr<RenderTextureManager> renderTextureManager_;
		RenderTexture* sceneColor_ = nullptr;
		RenderTexture* gBufferAlbedo_ = nullptr;
		RenderTexture* gBufferNormal_ = nullptr;
		RenderTexture* gBufferMaterial_ = nullptr;
		ComPtr<ID3D12Resource> toneMappingResource_ = nullptr;
		ToneMappingForGPU* toneMappingForGPU_ = nullptr;

		Window& window_;
	};

}
