#pragma once

#include "LGF/Graphics/Transform3D.h"
#include "LGF/Graphics/Color.h"

namespace LGF {

	class Triangle3D;

	namespace Graphics3D {

		void Draw(
			const Triangle3D& triangle,
			const Transform3D& transform = {},
			const Vector4& color = Color::White
		);

	}

}