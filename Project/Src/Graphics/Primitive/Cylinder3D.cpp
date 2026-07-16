#include "LGF/Graphics/Cylinder3D.h"
#include "LGF/Graphics/Graphics3D.h"

using namespace LGF;

Cylinder3D::Cylinder3D() {
}

Cylinder3D::Cylinder3D(float radius, float height) :
	radius(radius),
	height(height) {
}

Cylinder3D::~Cylinder3D() = default;

void Cylinder3D::Draw(const Matrix4x4& worldMatrix, const Vector4& color) const {
	Graphics3D::Draw(*this, worldMatrix, color);
}
