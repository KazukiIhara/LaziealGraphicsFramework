#pragma once

#include "LGF/Graphics/Color.h"
#include "LGF/Graphics/Texture.h"
#include "LGF/Math/MathTypes.h"

namespace LGF {

	// カメラに正対する加算合成の板。sizeはワールド単位、rotationはラジアン。
	struct Billboard3D {
		Vector3 position{};
		Vector2 size{ 1.0f, 1.0f };
		float rotation{};

		void Draw(const Vector4& color = Color::White, const Texture& texture = {}) const;
	};

}
