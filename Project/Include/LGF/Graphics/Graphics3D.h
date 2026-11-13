#pragma once

#include <cstdint>
#include <span>

#include "LGF/Math/Matrix4x4.h"
#include "LGF/Graphics/Color.h"
#include "LGF/Graphics/Material.h"
#include "LGF/Graphics/GpuParticle3D.h"

namespace LGF {

	class Camera3D;
	struct Billboard3D;
	class DirectionalLight;
	class PointLight;
	class Skybox;

	class Box3D;
	class Capsule3D;
	class Cone3D;
	class Cylinder3D;
	class Ground3D;
	class Line3D;
	class Model;
	class ModelAnimator;
	struct ModelDrawOptions;
	class Plane3D;
	class UIPanel3D;
	class Sphere3D;
	class SphericalCap3D;
	class SphericalShellCap3D;
	class Triangle3D;

	namespace Graphics3D {

		void DrawBillboard(const Billboard3D& billboard, const Vector4& color, const Texture& texture);

		GpuParticleWorldHandle CreateGpuParticleWorld(const GpuParticleWorldDesc& desc);
		bool RemoveGpuParticleWorld(GpuParticleWorldHandle handle);
		bool ClearGpuParticleWorld(GpuParticleWorldHandle handle);
		bool UpdateGpuParticleWorld(GpuParticleWorldHandle handle, float deltaTime, std::span<const GpuParticleSpawnBatch3D> spawnBatches);
		bool DrawGpuParticleWorld(GpuParticleWorldHandle handle);
		GpuParticleStatistics3D GetGpuParticleStatistics(GpuParticleWorldHandle handle);

		uint32_t GetWindowWidth();
		uint32_t GetWindowHeight();

		void SetCamera(const Camera3D& camera);
		void SetCullingCamera(const Camera3D& camera);
		void SetShadowCamera(const Camera3D& camera);
		void SetDirectionalLight(const DirectionalLight& light);
		void SetPointLight(const PointLight& light);
		void SetSkybox(const Skybox& skybox);
		/// <summary>トーンマッピング前の露出をEV単位で設定する。+1で2倍、-1で半分。</summary>
		void SetExposure(float exposureEV);

		void Draw(const Triangle3D& triangle, const Matrix4x4& worldMatrix, const Vector4& color = Color::White);
		void Draw(const Box3D& box, const Matrix4x4& worldMatrix, const Vector4& color = Color::White);
		void Draw(const Capsule3D& capsule, const Matrix4x4& worldMatrix, const Vector4& color = Color::White);
		void Draw(const Cone3D& cone, const Matrix4x4& worldMatrix, const Vector4& color = Color::White);
		void Draw(const Cylinder3D& cylinder, const Matrix4x4& worldMatrix, const Vector4& color = Color::White);
		void Draw(const Line3D& line, const Matrix4x4& worldMatrix, const Vector4& color = Color::White);
		void Draw(const Plane3D& plane, const Matrix4x4& worldMatrix, const Vector4& color = Color::White);
		void Draw(const UIPanel3D& panel, const Matrix4x4& worldMatrix, const Vector4& color = Color::White);
		void Draw(const Ground3D& ground, const Matrix4x4& worldMatrix, const Vector4& color = Color::White);
		void Draw(const Sphere3D& sphere, const Matrix4x4& worldMatrix, const Vector4& color = Color::White);
		void Draw(const SphericalCap3D& sphericalCap, const Matrix4x4& worldMatrix, const Vector4& color = Color::White);
		void Draw(const SphericalShellCap3D& sphericalShellCap, const Matrix4x4& worldMatrix, const Vector4& color = Color::White);
		void Draw(const Model& model, const Matrix4x4& worldMatrix, const Vector4& color, const ModelDrawOptions& options);
		void Draw(
			const Model& model,
			const ModelAnimator& animator,
			const Matrix4x4& worldMatrix,
			const Vector4& color,
			const ModelDrawOptions& options);

		void Draw(const Triangle3D& shape, const Matrix4x4& worldMatrix, const Material3D& material);
		void Draw(const Box3D& shape, const Matrix4x4& worldMatrix, const Material3D& material);
		void Draw(const Capsule3D& shape, const Matrix4x4& worldMatrix, const Material3D& material);
		void Draw(const Cone3D& shape, const Matrix4x4& worldMatrix, const Material3D& material);
		void Draw(const Cylinder3D& shape, const Matrix4x4& worldMatrix, const Material3D& material);
		void Draw(const Line3D& shape, const Matrix4x4& worldMatrix, const Material3D& material);
		void Draw(const Plane3D& shape, const Matrix4x4& worldMatrix, const Material3D& material);
		void Draw(const Ground3D& shape, const Matrix4x4& worldMatrix, const Material3D& material);
		void Draw(const Sphere3D& shape, const Matrix4x4& worldMatrix, const Material3D& material);
		void Draw(const SphericalCap3D& shape, const Matrix4x4& worldMatrix, const Material3D& material);
		void Draw(const SphericalShellCap3D& shape, const Matrix4x4& worldMatrix, const Material3D& material);

	}

}
