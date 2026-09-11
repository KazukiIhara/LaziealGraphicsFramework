#include "LGF/Graphics/Line3D.h"
#include "LGF/Graphics/Graphics3D.h"

using namespace LGF;

Line3D::Line3D() :
	start{ 0.0f, 0.0f, 0.0f },
	end{ 1.0f, 0.0f, 0.0f } {
}

Line3D::Line3D(const Vector3& start, const Vector3& end) :
	start(start),
	end(end) {
}

Line3D::Line3D(float x0, float y0, float z0, float x1, float y1, float z1) :
	start{ x0, y0, z0 },
	end{ x1, y1, z1 } {
}

Line3D::~Line3D() = default;

void Line3D::Draw(const Matrix4x4& worldMatrix, const Vector4& color) const {
	Graphics3D::Draw(*this, worldMatrix, color);
}

void Line3D::Draw(const Matrix4x4& worldMatrix, const Material3D& material) const {
	Graphics3D::Draw(*this, worldMatrix, material);
}
