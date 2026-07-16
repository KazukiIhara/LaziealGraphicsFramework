#pragma once

#include <memory>
#include <vector>

#include "Graphics/System/IGraphicsSubsystem.h"

#include "LGF/Math/MathTypes.h"

#include "Graphics/Graphics3DContext.h"

namespace LGF {

	class IDrawer;
	class BoxDrawer3D;
	class CapsuleDrawer3D;
	class ConeDrawer3D;
	class CylinderDrawer3D;
	class GroundDrawer3D;
	class LineDrawer3D;
	class PlaneDrawer3D;
	class SphereDrawer3D;
	class TriangleDrawer3D;
	class Box3D;
	class Capsule3D;
	class Cone3D;
	class Cylinder3D;
	class Ground3D;
	class Line3D;
	class Plane3D;
	class Sphere3D;
	class Triangle3D;

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
		void Finalize()override;

		// 各Drawerに描画コマンドを積む関数群

		// 三角形描画
		void AddTriangle(
			const Triangle3D& triangle,
			const Matrix4x4& worldMatrix,
			const Vector4& color
		);

		void AddBox(
			const Box3D& box,
			const Matrix4x4& worldMatrix,
			const Vector4& color
		);

		void AddCapsule(
			const Capsule3D& capsule,
			const Matrix4x4& worldMatrix,
			const Vector4& color
		);

		void AddCone(
			const Cone3D& cone,
			const Matrix4x4& worldMatrix,
			const Vector4& color
		);

		void AddCylinder(
			const Cylinder3D& cylinder,
			const Matrix4x4& worldMatrix,
			const Vector4& color
		);

		void AddLine(
			const Line3D& line,
			const Matrix4x4& worldMatrix,
			const Vector4& color
		);

		void AddPlane(
			const Plane3D& plane,
			const Matrix4x4& worldMatrix,
			const Vector4& color
		);

		void AddGround(
			const Ground3D& ground,
			const Matrix4x4& worldMatrix,
			const Vector4& color
		);

		void AddSphere(
			const Sphere3D& sphere,
			const Matrix4x4& worldMatrix,
			const Vector4& color
		);

	private:
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
		std::unique_ptr<PlaneDrawer3D> planeDrawer_;
		std::unique_ptr<SphereDrawer3D> sphereDrawer_;
		std::unique_ptr<TriangleDrawer3D> triangleDrawer_;
	};

}
