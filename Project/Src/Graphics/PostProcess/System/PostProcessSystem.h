#pragma once

#include <filesystem>
#include <memory>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Graphics/DirectX/FrameContext/FrameContext.h"
#include "Graphics/PostProcess/PostProcessTypes.h"

namespace LGF {

	class DirectXCore;
	class Pipeline;
	class PipelineManager;
	class RenderTexture;

	class PostProcessSystem {
	public:
		PostProcessSystem(DirectXCore& directX, PipelineManager& pipelineManager, std::filesystem::path assetRoot);
		~PostProcessSystem();

		bool Initialize();
		void Finalize();
		bool Resize(uint32_t width, uint32_t height);
		bool Execute();

		PostProcessProfileHandle CreateProfile(const std::filesystem::path& filePath);
		bool RemoveProfile(PostProcessProfileHandle handle);
		bool IsValid(PostProcessProfileHandle handle) const;
		bool SetActiveProfile(PostProcessProfileHandle handle);
		void ResetActiveProfile();

		bool SetEffectEnabled(PostProcessProfileHandle handle, std::string_view effectId, bool enabled);
		bool SetParameter(PostProcessProfileHandle handle, std::string_view effectId, std::string_view parameterName, float value);
		bool SetParameter(PostProcessProfileHandle handle, std::string_view effectId, std::string_view parameterName, const Vector2& value);
		bool SetParameter(PostProcessProfileHandle handle, std::string_view effectId, std::string_view parameterName, const Vector3& value);
		bool SetParameter(PostProcessProfileHandle handle, std::string_view effectId, std::string_view parameterName, const Vector4& value);
		bool ClearParameterOverride(PostProcessProfileHandle handle, std::string_view effectId, std::string_view parameterName);
		bool ClearOverrides(PostProcessProfileHandle handle);
		void SetExposure(float exposureEV);

	private:
		struct EffectInstance {
			std::string id;
			const PostEffectDefinition* definition = nullptr;
			PostProcessParametersForGPU baseParameters{};
			PostProcessParametersForGPU parameters{};
			std::unique_ptr<FrameBufferedUploadBuffer> parameterResources;
			std::unordered_set<std::string> overriddenParameters;
			bool baseEnabled = true;
			bool enabled = true;
		};

		struct ProfileInstance {
			std::string name;
			std::vector<EffectInstance> effects;
		};

		struct ProfileSlot {
			std::unique_ptr<ProfileInstance> instance;
			uint32_t generation = 1u;
		};

		bool CreateRenderTextures(uint32_t width, uint32_t height);
		std::unique_ptr<RenderTexture> CreateRenderTexture(uint32_t width, uint32_t height, bool hdr);
		ProfileInstance* FindProfile(PostProcessProfileHandle handle);
		const ProfileInstance* FindProfile(PostProcessProfileHandle handle) const;
		EffectInstance* FindEffect(PostProcessProfileHandle handle, std::string_view effectId);
		bool SetParameter(PostProcessProfileHandle handle, std::string_view effectId, std::string_view parameterName, const float* values, uint32_t valueCount);
		bool ExecuteProfile(ProfileInstance& profile);
		bool ExecuteFallback();
		bool ExecuteFullscreenEffect(EffectInstance& effect, RenderTexture& input, RenderTexture& output);
		bool ExecuteBloom(EffectInstance& effect, RenderTexture& input, RenderTexture& output);
		bool ExecuteToneMapping(EffectInstance& effect, RenderTexture& input, RenderTexture& output);
		bool DrawFullscreen(Pipeline& pipeline, RenderTexture& input, RenderTexture& output, D3D12_GPU_VIRTUAL_ADDRESS parameterAddress);
		bool Present(RenderTexture& input);
		void BindSceneInputs(Pipeline& pipeline, RenderTexture& input);
		void UpdateContext(RenderTexture& input, RenderTexture& output);
		Pipeline* FindPipeline(std::string_view name) const;

	private:
		DirectXCore& directX_;
		PipelineManager& pipelineManager_;
		std::filesystem::path assetRoot_;
		std::unordered_map<std::string, PostEffectDefinition> definitions_;
		std::vector<ProfileSlot> profiles_;
		std::vector<uint32_t> freeProfileIndices_;
		PostProcessProfileHandle activeProfile_{};

		std::unique_ptr<RenderTexture> hdrPing_;
		std::unique_ptr<RenderTexture> hdrPong_;
		std::unique_ptr<RenderTexture> ldrPing_;
		std::unique_ptr<RenderTexture> ldrPong_;
		std::unique_ptr<RenderTexture> bloomPing_;
		std::unique_ptr<RenderTexture> bloomPong_;
		FrameBufferedUploadBuffer contextResources_;
		FrameBufferedUploadBuffer fallbackParameterResources_;
		PostProcessParametersForGPU fallbackParameters_{};
		float exposureEV_ = 0.0f;
		bool initialized_ = false;
	};

}
