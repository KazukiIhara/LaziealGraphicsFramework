#pragma once

#include "LGF/Math/Matrix4x4.h"
#include "LGF/Graphics/Color.h"

namespace LGF {

	class Triangle3D;

	namespace Graphics3D {

		void Draw(
			const Triangle3D& triangle,
			const Matrix4x4& worldMatrix,
			const Vector4& color = Color::White
		);

	}

}