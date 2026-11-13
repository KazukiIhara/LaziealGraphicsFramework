#pragma once

#include <filesystem>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "LGF/Math/MathUtils.h"
#include "LGF/Math/Vector4.h"
#include "Color.h"

namespace LGF {
	struct ModelDebugDraw {
		bool showMeshletColors = false;
		bool showMeshletBoundingSpheres = false;
		Vector4 meshletBoundingSphereColor = Color::Lime;
	};

	/// <summary>
	/// モデル描画ごとの設定。今後の描画機能はこの構造体へ追加する。
	/// </summary>
	struct ModelDrawOptions {
		Vector4 color = Color::White;
		bool enableNormalMap = true;
		float normalScale = 1.0f;
		float metallicScale = 1.0f;
		float roughnessScale = 1.0f;
		float occlusionStrength = 1.0f;
		bool unlit = false;
		ModelDebugDraw debugDraw{};
	};

	struct ModelInstance3D {
		Matrix4x4 worldMatrix;
		Vector4 color = Color::White;
	};

	struct ModelResource;
	struct ModelAnimatorResource;
	class Renderer3D;
	class ModelAnimator;

	/// <summary>
	/// ファイルから読み込んだ3Dモデル
	/// </summary>
	class Model {
	public:
		Model() = default;
		explicit Model(const std::filesystem::path& filePath);
		~Model();

		void Draw(
			const Matrix4x4& worldMatrix = Math::Identity4x4(),
			const ModelDrawOptions& options = {}
		) const;
		void Draw(
			const Matrix4x4& worldMatrix,
			const ModelAnimator& animator,
			const ModelDrawOptions& options = {}
		) const;

		void DrawInstanced(
			std::span<const ModelInstance3D> instances,
			const ModelDrawOptions& options = {}
		) const;
		void DrawInstanced(
			std::span<const ModelInstance3D> instances,
			const ModelAnimator& animator,
			const ModelDrawOptions& options = {}
		) const;

		const std::filesystem::path& GetFilePath() const;
		bool IsValid() const;

	private:
		friend class Renderer3D;
		friend class ModelAnimator;

		std::shared_ptr<ModelResource> resource_;
		std::filesystem::path filePath_;
	};

	struct ModelAnimationPlayOptions {
		bool loop = true;
		float playbackSpeed = 1.0f;
		float fadeDuration = 0.0f;
	};

	/// <summary>
	/// Modelごとのアニメーション再生状態。同じModelAnimatorを使った描画は同じ姿勢としてまとめられる。
	/// </summary>
	class ModelAnimator {
	public:
		ModelAnimator() = default;
		explicit ModelAnimator(const Model& model);
		~ModelAnimator();

		bool Play(std::string_view animationName, const ModelAnimationPlayOptions& options = {});
		void Stop();
		void Update(float deltaTime);

		std::vector<std::string> GetAnimationNames() const;
		std::string_view GetCurrentAnimationName() const;
		bool IsPlaying() const;
		bool IsValid() const;

	private:
		friend class Model;
		friend class Renderer3D;

		std::shared_ptr<ModelAnimatorResource> resource_;
	};

}
