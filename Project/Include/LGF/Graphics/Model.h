#pragma once

#include <memory>
#include <span>
#include <string>

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
		bool unlit = false;
		ModelDebugDraw debugDraw{};
	};

	struct ModelInstance3D {
		Matrix4x4 worldMatrix;
		Vector4 color = Color::White;
	};

	struct ModelResource;
	class Renderer3D;

	/// <summary>
	/// ファイルから読み込んだ静的3Dモデル
	/// </summary>
	class Model {
	public:
		Model() = default;
		explicit Model(const std::string& filePath);
		~Model();

		void Draw(
			const Matrix4x4& worldMatrix = Math::Identity4x4(),
			const ModelDrawOptions& options = {}
		) const;

		void DrawInstanced(
			std::span<const ModelInstance3D> instances,
			const ModelDrawOptions& options = {}
		) const;

		const std::string& GetFilePath() const;
		bool IsValid() const;

	private:
		friend class Renderer3D;

		std::shared_ptr<ModelResource> resource_;
		std::string filePath_;
	};

}
