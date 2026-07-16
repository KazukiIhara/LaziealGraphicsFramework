#include "LGF/Graphics/Sphere3D.h"
#include "LGF/Graphics/Graphics3D.h"

using namespace LGF;

Sphere3D::Sphere3D() {
}

Sphere3D::Sphere3D(float radius) :
	radius(radius) {
}

Sphere3D::~Sphere3D() = default;

void Sphere3D::Draw(const Matrix4x4& worldMatrix, const Vector4& color) const {
	Graphics3D::Draw(*this, worldMatrix, color);
}
