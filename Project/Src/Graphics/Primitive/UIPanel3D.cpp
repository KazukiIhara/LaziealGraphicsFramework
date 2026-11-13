#include "LGF/Graphics/UIPanel3D.h"

#include "LGF/Graphics/Graphics3D.h"

using namespace LGF;

UIPanel3D::UIPanel3D() {
}

UIPanel3D::UIPanel3D(float size) :
	width(size),
	height(size) {
}

UIPanel3D::UIPanel3D(float width, float height) :
	width(width),
	height(height) {
}

UIPanel3D::~UIPanel3D() = default;

void UIPanel3D::Draw(const Matrix4x4& worldMatrix, const Vector4& color) const {
	Graphics3D::Draw(*this, worldMatrix, color);
}
