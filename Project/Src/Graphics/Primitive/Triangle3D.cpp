#include "LGF/Graphics/Triangle3D.h"
#include "LGF/Graphics/Graphics3D.h"

using namespace LGF;

Triangle3D::Triangle3D() {
}

Triangle3D::Triangle3D(const Vector3& v0, const Vector3& v1, const Vector3& v2) :
	vertices{ v0, v1, v2 } {
}

Triangle3D::~Triangle3D() = default;

void Triangle3D::Draw(const Matrix4x4& worldMatrix, const Vector4& color) const {
	Graphics3D::Draw(*this, worldMatrix, color);
}
