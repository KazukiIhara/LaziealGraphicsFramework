#pragma once

#include <memory>
#include <span>
#include <string>

#include "LGF/Math/Matrix4x4.h"
#include "LGF/Math/Vector4.h"
#include "Color.h"

namespace LGF {
	struct ModelDebugDraw {
		bool showMeshletColors = false;
		bool showMeshletBoundingSpheres = false;
		Vector4 meshletBoundingSphereColor = Color::Lime;
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
			const Matrix4x4& worldMatrix,
			const Vector4& color = Color::White,
			const ModelDebugDraw& debugDraw = {}
			) const;

		void DrawInstanced(
			std::span<const ModelInstance3D> instances,
			const ModelDebugDraw& debugDraw = {}
			) const;

		const std::string& GetFilePath() const;
		bool IsValid() const;

	private:
		friend class Renderer3D;

		std::shared_ptr<ModelResource> resource_;
		std::string filePath_;
	};

}
