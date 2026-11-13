#pragma once

#include <memory>
#include <span>
#include <string_view>
#include <vector>

#include "Graphics/System/IGraphicsSubsystem.h"

#include "LGF/Math/MathTypes.h"
#include "LGF/Graphics/Material.h"
#include "LGF/Graphics/GpuParticle3D.h"

#include "Graphics/Graphics3DContext.h"

namespace LGF {

	struct InstanceData3D;
	class IDrawer;
	class BoxDrawer3D;
	class CapsuleDrawer3D;
	class ConeDrawer3D;
	class CylinderDrawer3D;
	class GroundDrawer3D;
	class LineDrawer3D;
	class MeshletBoundsDrawer3D;
	class ModelDrawer3D;
	class PlaneDrawer3D;
	class ParticleDrawer3D;
	class GpuParticleDrawer3D;
	class FontDrawer3D;
	struct Billboard3D;
	class Font3D;
	struct Font3DDrawOptions;
	class SphereDrawer3D;
	class SphericalCapDrawer3D;
	class SphericalShellCapDrawer3D;
	class TriangleDrawer3D;
	class Box3D;
	class Capsule3D;
	class Cone3D;
	class Cylinder3D;
	class Ground3D;
	class Line3D;
	class Model;
	struct ModelDrawOptions;
	class Plane3D;
	class UIPanel3D;
	class Sphere3D;
	class SphericalCap3D;
	class SphericalShellCap3D;
	class Triangle3D;
	class ShadowMapSystem;

	/// <summary>
	/// 全3D描画をするクラス
	/// </summary>
	class Renderer3D : public IGraphicsSubsystem {
	public:
		Renderer3D(Graphics3DContext context);
		~Renderer3D();

		bool Initialize()override;
		void BeginFrame()override;
		void EndFrame()override;
		void DrawShadow(ShadowMapSystem& shadowMap);
		void DrawTransparent();
		void AddBillboard(const Billboard3D& billboard, const Vector4& color, const Texture& texture);
		void AddFont(
			const Font3D& font,
			std::string_view text,
			const Vector3& position,
			const Font3DDrawOptions& options);
		GpuParticleWorldHandle CreateGpuParticleWorld(const GpuParticleWorldDesc& desc);
		bool RemoveGpuParticleWorld(GpuParticleWorldHandle handle);
		bool ClearGpuParticleWorld(GpuParticleWorldHandle handle);
		bool UpdateGpuParticleWorld(GpuParticleWorldHandle handle, float deltaTime, std::span<const GpuParticleSpawnBatch3D> spawnBatches);
		bool DrawGpuParticleWorld(GpuParticleWorldHandle handle);
		GpuParticleStatistics3D GetGpuParticleStatistics(GpuParticleWorldHandle handle) const;
		void Finalize()override;

		// 各Drawerに描画コマンドを積む関数群

		// 三角形描画
		void AddTriangle(const Triangle3D& triangle, const Matrix4x4& worldMatrix, const Material3D& material);
		void AddBox(const Box3D& box, const Matrix4x4& worldMatrix, const Material3D& material);
		void AddCapsule(const Capsule3D& capsule, const Matrix4x4& worldMatrix, const Material3D& material);
		void AddCone(const Cone3D& cone, const Matrix4x4& worldMatrix, const Material3D& material);
		void AddCylinder(const Cylinder3D& cylinder, const Matrix4x4& worldMatrix, const Material3D& material);
		void AddLine(const Line3D& line, const Matrix4x4& worldMatrix, const Material3D& material);
		void AddPlane(const Plane3D& plane, const Matrix4x4& worldMatrix, const Material3D& material);
		void AddUIPanel(const UIPanel3D& panel, const Matrix4x4& worldMatrix, const Material3D& material);
		void AddGround(const Ground3D& ground, const Matrix4x4& worldMatrix, const Material3D& material);
		void AddSphere(const Sphere3D& sphere, const Matrix4x4& worldMatrix, const Material3D& material);
		void AddSphericalCap(const SphericalCap3D& sphericalCap, const Matrix4x4& worldMatrix, const Material3D& material);
		void AddSphericalShellCap(const SphericalShellCap3D& sphericalShellCap, const Matrix4x4& worldMatrix, const Material3D& material);
		void AddModel(const Model& model, const Matrix4x4& worldMatrix, const Vector4& color, const ModelDrawOptions& options);

	private:
		static void ApplyMaterial(InstanceData3D& instance, const Material3D& material);
		// 参照
		Graphics3DContext context_;
		// 全更新用リスト
		std::vector<IDrawer*> drawerList_;

		// 描画クラス
		std::unique_ptr<BoxDrawer3D> boxDrawer_;
		std::unique_ptr<CapsuleDrawer3D> capsuleDrawer_;
		std::unique_ptr<ConeDrawer3D> coneDrawer_;
		std::unique_ptr<CylinderDrawer3D> cylinderDrawer_;
		std::unique_ptr<GroundDrawer3D> groundDrawer_;
		std::unique_ptr<LineDrawer3D> lineDrawer_;
		std::unique_ptr<MeshletBoundsDrawer3D> meshletBoundsDrawer_;
		std::unique_ptr<ModelDrawer3D> modelDrawer_;
		std::unique_ptr<PlaneDrawer3D> planeDrawer_;
		std::unique_ptr<PlaneDrawer3D> uiPanelDrawer_;
		std::unique_ptr<ParticleDrawer3D> particleDrawer_;
		std::unique_ptr<GpuParticleDrawer3D> gpuParticleDrawer_;
		std::unique_ptr<FontDrawer3D> fontDrawer_;
		std::unique_ptr<SphereDrawer3D> sphereDrawer_;
		std::unique_ptr<SphericalCapDrawer3D> sphericalCapDrawer_;
		std::unique_ptr<SphericalShellCapDrawer3D> sphericalShellCapDrawer_;
		std::unique_ptr<TriangleDrawer3D> triangleDrawer_;
	};

}
