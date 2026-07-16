#include "LGF/Graphics/Box3D.h"
#include "LGF/Graphics/Graphics3D.h"

using namespace LGF;

Box3D::Box3D() {
}

Box3D::Box3D(float size) :
	width(size),
	height(size),
	depth(size) {
}

Box3D::Box3D(float width, float height, float depth) :
	width(width),
	height(height),
	depth(depth) {
}

Box3D::~Box3D() = default;

void Box3D::Draw(const Matrix4x4& worldMatrix, const Vector4& color) const {
	Graphics3D::Draw(*this, worldMatrix, color);
}
