#include "LGF/Graphics/Transform3D.h"

#include "LGF/Math/MathUtils.h"

using namespace LGF;

Transform3D::Transform3D(const Vector3& scale, const Quaternion& rotation, const Vector3& translation) :
	scale(scale), rotation(rotation), translation(translation) {
}

Transform3D::Transform3D(const Vector3& translation) :
	translation(translation) {
}

Transform3D::~Transform3D() = default;

void Transform3D::Update() {
	worldMatrix_ = Math::MakeAffineMatrix(scale, rotation, translation);
	if (parent_) {
		worldMatrix_ = worldMatrix_ * parent_->GetWorldMatrix();
	}

	if (!children_.empty()) {
		for (auto& child : children_) {
			if (child) {
				child->Update();
			}
		}
	}
}

void Transform3D::Finalize() {

}

 Matrix4x4 Transform3D::GetWorldMatrix() const {
	return worldMatrix_;
}

 Vector3 Transform3D::GetWorldPosition() const {
	return worldPosition_;
}

void Transform3D::SetParent(Transform3D* parent) {

}
