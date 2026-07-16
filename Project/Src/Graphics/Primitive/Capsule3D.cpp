#include "LGF/Graphics/Capsule3D.h"
#include "LGF/Graphics/Graphics3D.h"

using namespace LGF;

Capsule3D::Capsule3D() {
}

Capsule3D::Capsule3D(float radius, float height) :
	radius(radius),
	height(height) {
}

Capsule3D::~Capsule3D() = default;

void Capsule3D::Draw(const Matrix4x4& worldMatrix, const Vector4& color) const {
	Graphics3D::Draw(*this, worldMatrix, color);
}
