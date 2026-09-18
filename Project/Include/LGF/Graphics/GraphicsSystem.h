#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <vector>

namespace LGF {

	class IGraphicsSubsystem;
	class DirectXCore;
	class PipelineManager;
	class Camera3DSystem;
	class DirectionalLightSystem;
	class SkyboxSystem;
	class ShadowMapSystem;
	class Renderer3D;
	class Camera2DSystem;
	class Renderer2D;
	class TextureManager;
	class ModelManager;
	class ImGuiSystem;

	struct GraphicsInitDesc {
		void* windowHandle = nullptr;
		uint32_t width = 0;
		uint32_t height = 0;
		std::filesystem::path assetRoot;
	};

	/// <summary>
	/// 描画フレームワークのライフサイクルを管理するクラス。
	/// Runtime層からネイティブウィンドウ情報を受け取り、Window/Inputには依存しない。
	/// </summary>
	class GraphicsSystem {
	public:
		GraphicsSystem();
		~GraphicsSystem();

		bool Initialize(const GraphicsInitDesc& desc);
		bool Finalize();
		bool Resize(uint32_t width, uint32_t height);

		void BeginFrame();
		void EndFrame();

		bool HandleWindowMessage(
			void* windowHandle,
			uint32_t message,
			uintptr_t wParam,
			intptr_t lParam);

	private:
		std::vector<IGraphicsSubsystem*> subsystems_;
		std::unique_ptr<DirectXCore> directX_;
		std::unique_ptr<PipelineManager> pipelineManager_;
		std::unique_ptr<TextureManager> textureManager_;
		std::unique_ptr<ModelManager> modelManager_;
		std::unique_ptr<Camera3DSystem> camera3DSystem_;
		std::unique_ptr<DirectionalLightSystem> directionalLightSystem_;
		std::unique_ptr<SkyboxSystem> skyboxSystem_;
		std::unique_ptr<ShadowMapSystem> shadowMapSystem_;
		std::unique_ptr<Renderer3D> renderer3D_;
		std::unique_ptr<Camera2DSystem> camera2DSystem_;
		std::unique_ptr<Renderer2D> renderer2D_;
		std::unique_ptr<ImGuiSystem> imguiSystem_;
		bool isFrameRenderable_ = false;
		bool isLoggerInitialized_ = false;
	};

}
