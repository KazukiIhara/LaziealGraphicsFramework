#include "LGF/Graphics/SphericalShellCap3D.h"
#include "LGF/Graphics/Graphics3D.h"

using namespace LGF;

SphericalShellCap3D::SphericalShellCap3D() {
}

SphericalShellCap3D::SphericalShellCap3D(
	float outerRadius,
	float innerRadius,
	float height) :
	outerRadius(outerRadius),
	innerRadius(innerRadius),
	height(height) {
}

SphericalShellCap3D::~SphericalShellCap3D() = default;

void SphericalShellCap3D::Draw(
	const Matrix4x4& worldMatrix,
	const Vector4& color) const {
	Graphics3D::Draw(*this, worldMatrix, color);
}

void SphericalShellCap3D::Draw(
	const Matrix4x4& worldMatrix,
	const Material3D& material) const {
	Graphics3D::Draw(*this, worldMatrix, material);
}
