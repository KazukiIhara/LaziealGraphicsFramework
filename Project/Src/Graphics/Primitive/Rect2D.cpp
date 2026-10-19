#include "LGF/Graphics/Rect2D.h"
#include "LGF/Graphics/Graphics2D.h"
#include "LGF/Graphics/Material.h"

using namespace LGF;

Rect2D::Rect2D() :
	leftTop{ 0.0f, 0.0f },
	size{ 100.0f, 100.0f } {
}

Rect2D::Rect2D(const Vector2& leftTop, const Vector2& size) :
	leftTop(leftTop),
	size(size) {
}

Rect2D::Rect2D(float x, float y, float width, float height) :
	leftTop{ x, y },
	size{ width, height } {
}

Rect2D::~Rect2D() = default;

void Rect2D::Draw(const Vector4& color) const {
	Graphics2D::Draw(*this, color);
}

void Rect2D::Draw(const Material2D& material) const {
	Graphics2D::Draw(*this, material);
}
