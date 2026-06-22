#include "Triangle3D.h"

using namespace LGF;

Triangle3D::Triangle3D() {
	vertices = {};
}

Triangle3D::Triangle3D(const Vector3& v0, const Vector3& v1, const Vector3& v2) :
	vertices{ v0, v1, v2 } {
}

Triangle3D::~Triangle3D() = default;
