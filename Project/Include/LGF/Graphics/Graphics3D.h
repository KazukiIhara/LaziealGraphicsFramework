#pragma once

#include <cstdint>

#include "LGF/Math/Matrix4x4.h"
#include "LGF/Graphics/Color.h"

namespace LGF {

	class Camera3D;

	class Box3D;
	class Ground3D;
	class Line3D;
	class Plane3D;
	class Sphere3D;
	class Triangle3D;

	namespace Graphics3D {

		uint32_t GetWindowWidth();
		uint32_t GetWindowHeight();

		/// <summary>
		/// Set active camera.
		/// </summary>
		void SetCamera(const Camera3D& camera);

		/// <summary>
		/// Draw a triangle.
		/// </summary>
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

	}

}
