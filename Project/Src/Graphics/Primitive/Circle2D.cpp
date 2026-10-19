#include "LGF/Graphics/Circle2D.h"
#include "LGF/Graphics/Graphics2D.h"
#include "LGF/Graphics/Material.h"

using namespace LGF;

Circle2D::Circle2D() :
	center{ 50.0f, 50.0f },
	radius{ 50.0f } {
}

Circle2D::Circle2D(const Vector2& center, float radius) :
	center(center),
	radius(radius) {
}

Circle2D::Circle2D(float x, float y, float radius) :
	center{ x, y },
	radius(radius) {
}

Circle2D::~Circle2D() = default;

void Circle2D::Draw(const Vector4& color) const {
	Graphics2D::Draw(*this, color);
}

void Circle2D::Draw(const Material2D& material) const {
	Graphics2D::Draw(*this, material);
}
