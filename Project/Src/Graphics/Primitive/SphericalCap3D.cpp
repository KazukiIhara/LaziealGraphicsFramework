#include "LGF/Graphics/SphericalCap3D.h"
#include "LGF/Graphics/Graphics3D.h"

using namespace LGF;

SphericalCap3D::SphericalCap3D() {
}

SphericalCap3D::SphericalCap3D(float baseRadius, float height) :
	baseRadius(baseRadius),
	height(height) {
}

SphericalCap3D::~SphericalCap3D() = default;

void SphericalCap3D::Draw(const Matrix4x4& worldMatrix, const Vector4& color) const {
	Graphics3D::Draw(*this, worldMatrix, color);
}

void SphericalCap3D::Draw(
	const Matrix4x4& worldMatrix,
	const Material3D& material) const {
	Graphics3D::Draw(*this, worldMatrix, material);
}
