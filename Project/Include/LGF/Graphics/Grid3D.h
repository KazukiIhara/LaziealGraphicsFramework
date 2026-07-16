#pragma once

#include "LGF/Math/MathTypes.h"
#include "LGF/Math/MathUtils.h"

namespace LGF {

	class Grid3D {
	public:
		Grid3D();
		Grid3D(float size, float gridSpacing);

		~Grid3D();

		void Draw(
			const Matrix4x4& worldMatrix = Math::Identity4x4()
		)const;

	public:
		float size = 10.0f;
		float gridSpacing = 1.0f;
	};

}
