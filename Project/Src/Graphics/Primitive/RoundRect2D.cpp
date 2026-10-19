#include "LGF/Graphics/RoundRect2D.h"
#include "LGF/Graphics/Graphics2D.h"
#include "LGF/Graphics/Material.h"

using namespace LGF;

RoundRect2D::RoundRect2D() :
	leftTop{ 0.0f, 0.0f },
	size{ 100.0f, 100.0f },
	radius{ 16.0f } {
}

RoundRect2D::RoundRect2D(const Vector2& leftTop, const Vector2& size, float radius) :
	leftTop(leftTop),
	size(size),
	radius(radius) {
}

RoundRect2D::RoundRect2D(float x, float y, float width, float height, float radius) :
	leftTop{ x, y },
	size{ width, height },
	radius(radius) {
}

RoundRect2D::~RoundRect2D() = default;

void RoundRect2D::Draw(const Vector4& color) const {
	Graphics2D::Draw(*this, color);
}

void RoundRect2D::Draw(const Material2D& material) const {
	Graphics2D::Draw(*this, material);
}
