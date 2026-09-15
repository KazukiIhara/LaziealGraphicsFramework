#pragma once

#include <cstdint>

#include "LGF/Math/Matrix4x4.h"
#include "LGF/Graphics/Color.h"
#include "LGF/Graphics/Material.h"

namespace LGF {

	class Camera3D;
	class DirectionalLight;
	class Skybox;

	class Box3D;
	class Capsule3D;
	class Cone3D;
	class Cylinder3D;
	class Ground3D;
	class Line3D;
	class Model;
	struct ModelDrawOptions;
	class Plane3D;
	class Sphere3D;
	class Triangle3D;

	namespace Graphics3D {

		uint32_t GetWindowWidth();
		uint32_t GetWindowHeight();

		void SetCamera(const Camera3D& camera);
		void SetCullingCamera(const Camera3D& camera);
		void SetDirectionalLight(const DirectionalLight& light);
		void SetSkybox(const Skybox& skybox);
		/// <summary>トーンマッピング前の露出をEV単位で設定する。+1で2倍、-1で半分。</summary>
		void SetExposure(float exposureEV);

		void Draw(
			const Triangle3D& triangle,
			const Matrix4x4& worldMatrix,
			const Vector4& color = Color::White
		);

		void Draw(
			const Box3D& box,
			const Matrix4x4& worldMatrix,
			const Vector4& color = Color::White
		);

		void Draw(
			const Capsule3D& capsule,
			const Matrix4x4& worldMatrix,
			const Vector4& color = Color::White
		);

		void Draw(
			const Cone3D& cone,
			const Matrix4x4& worldMatrix,
			const Vector4& color = Color::White
		);

		void Draw(
			const Cylinder3D& cylinder,
			const Matrix4x4& worldMatrix,
			const Vector4& color = Color::White
		);

		void Draw(
			const Line3D& line,
			const Matrix4x4& worldMatrix,
			const Vector4& color = Color::White
		);

		void Draw(
			const Plane3D& plane,
			const Matrix4x4& worldMatrix,
			const Vector4& color = Color::White
		);

		void Draw(
			const Ground3D& ground,
			const Matrix4x4& worldMatrix,
			const Vector4& color = Color::White
		);

		void Draw(
			const Sphere3D& sphere,
			const Matrix4x4& worldMatrix,
			const Vector4& color = Color::White
		);

		void Draw(
			const Model& model,
			const Matrix4x4& worldMatrix,
			const Vector4& color,
			const ModelDrawOptions& options
			);

		void Draw(const Triangle3D& shape, const Matrix4x4& worldMatrix, const Material3D& material);
		void Draw(const Box3D& shape, const Matrix4x4& worldMatrix, const Material3D& material);
		void Draw(const Capsule3D& shape, const Matrix4x4& worldMatrix, const Material3D& material);
		void Draw(const Cone3D& shape, const Matrix4x4& worldMatrix, const Material3D& material);
		void Draw(const Cylinder3D& shape, const Matrix4x4& worldMatrix, const Material3D& material);
		void Draw(const Line3D& shape, const Matrix4x4& worldMatrix, const Material3D& material);
		void Draw(const Plane3D& shape, const Matrix4x4& worldMatrix, const Material3D& material);
		void Draw(const Ground3D& shape, const Matrix4x4& worldMatrix, const Material3D& material);
		void Draw(const Sphere3D& shape, const Matrix4x4& worldMatrix, const Material3D& material);

	}

}
