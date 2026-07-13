#include "LGF/Graphics/Line2D.h"
#include "LGF/Graphics/Graphics2D.h"

using namespace LGF;

Line2D::Line2D() :
	start{ 0.0f, 0.0f },
	end{ 100.0f, 100.0f },
	thickness{ 1.0f } {
}

Line2D::Line2D(const Vector2& start, const Vector2& end, float thickness) :
	start(start),
	end(end),
	thickness(thickness) {
}

Line2D::Line2D(float x0, float y0, float x1, float y1, float thickness) :
	start{ x0, y0 },
	end{ x1, y1 },
	thickness(thickness) {
}

Line2D::~Line2D() = default;

void Line2D::Draw(const Vector4& color) const {
	Graphics2D::Draw(*this, color);
}
