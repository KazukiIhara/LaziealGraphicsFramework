#include "LGF/Graphics/Ground3D.h"
#include "LGF/Graphics/Graphics3D.h"

using namespace LGF;

Ground3D::Ground3D() {
}

Ground3D::Ground3D(float size) :
	width(size),
	depth(size) {
}

Ground3D::Ground3D(float width, float depth) :
	width(width),
	depth(depth) {
}

Ground3D::~Ground3D() = default;

void Ground3D::Draw(const Matrix4x4& worldMatrix, const Vector4& color) const {
	Graphics3D::Draw(*this, worldMatrix, color);
}
