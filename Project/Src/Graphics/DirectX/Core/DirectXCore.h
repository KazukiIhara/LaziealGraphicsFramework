#pragma once

#include <cstdint>
#include <memory>

#include <d3d12.h>

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

	/// <summary>
	/// DirectXコアシステム
	/// </summary>
	class DirectXCore {
	public:
		DirectXCore();
		~DirectXCore();

		bool Initialize(Window& window);
		void Finalize();

		void BeginFrame();
		void EndFrame();

		ID3D12Device* GetDevice();
		ID3D12GraphicsCommandList* GetCommandList();

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
	};

}
