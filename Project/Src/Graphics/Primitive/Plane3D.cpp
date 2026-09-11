#include "LGF/Graphics/Plane3D.h"
#include "LGF/Graphics/Graphics3D.h"

using namespace LGF;

Plane3D::Plane3D() {
}

Plane3D::Plane3D(float size) :
	width(size),
	height(size) {
}

Plane3D::Plane3D(float width, float height) :
	width(width),
	height(height) {
}

Plane3D::~Plane3D() = default;

void Plane3D::Draw(const Matrix4x4& worldMatrix, const Vector4& color) const {
	Graphics3D::Draw(*this, worldMatrix, color);
}

void Plane3D::Draw(const Matrix4x4& worldMatrix, const Material3D& material) const {
	Graphics3D::Draw(*this, worldMatrix, material);
}
