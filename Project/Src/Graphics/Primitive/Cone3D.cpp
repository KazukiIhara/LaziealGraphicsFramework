#include "LGF/Graphics/Cone3D.h"
#include "LGF/Graphics/Graphics3D.h"

using namespace LGF;

Cone3D::Cone3D() {
}

Cone3D::Cone3D(float radius, float height) :
	radius(radius),
	height(height) {
}

Cone3D::~Cone3D() = default;

void Cone3D::Draw(const Matrix4x4& worldMatrix, const Vector4& color) const {
	Graphics3D::Draw(*this, worldMatrix, color);
}
