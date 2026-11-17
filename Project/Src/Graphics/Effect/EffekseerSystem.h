#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>

#include "Graphics/System/IGraphicsSubsystem.h"
#include "LGF/Graphics/Effect3D.h"

namespace LGF {

	class Camera3DSystem;
	class DirectXCore;

	class EffekseerSystem final : public IGraphicsSubsystem {
	public:
		EffekseerSystem(
			DirectXCore& directX,
			Camera3DSystem& cameraSystem,
			std::filesystem::path assetRoot);
		~EffekseerSystem() override;

		bool Initialize() override;
		void BeginFrame() override;
		void EndFrame() override;
		void Finalize() override;

		EffectWorldHandle3D CreateWorld(uint32_t maximumInstanceCount);
		bool RemoveWorld(EffectWorldHandle3D handle);
		EffectResourceHandle3D Load(
			EffectWorldHandle3D world,
			const std::filesystem::path& filePath);
		bool IsValid(EffectResourceHandle3D handle) const;
		bool Remove(EffectResourceHandle3D handle);
		EffectInstanceHandle3D Play(
			EffectResourceHandle3D effect,
			const Matrix4x4& transform);
		bool Stop(EffectInstanceHandle3D handle);
		bool IsPlaying(EffectInstanceHandle3D handle) const;
		bool SetTransform(
			EffectInstanceHandle3D handle,
			const Matrix4x4& transform);
		bool Update(EffectWorldHandle3D handle, float deltaTime);
		bool Clear(EffectWorldHandle3D handle);
		void Draw();

	private:
		class Impl;

		DirectXCore& directX_;
		Camera3DSystem& cameraSystem_;
		std::filesystem::path assetRoot_;
		std::unique_ptr<Impl> impl_;
	};

}
