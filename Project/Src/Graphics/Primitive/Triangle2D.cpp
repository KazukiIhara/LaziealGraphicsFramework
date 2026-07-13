#include "LGF/Graphics/Triangle2D.h"
#include "LGF/Graphics/Graphics2D.h"

using namespace LGF;

Triangle2D::Triangle2D()
	: vertices{
		Vector2{50.0f, 0.0f},
		Vector2{100.0f, 100.0f},
		Vector2{0.0f, 100.0f}
	} {
}

Triangle2D::Triangle2D(const Vector2& v0, const Vector2& v1, const Vector2& v2) :
	vertices{ v0, v1, v2 } {
}

Triangle2D::~Triangle2D() = default;

void Triangle2D::Draw(const Vector4& color) const {
	Graphics2D::Draw(*this, color);
}
