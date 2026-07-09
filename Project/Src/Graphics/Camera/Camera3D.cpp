#include "LGF/Graphics/Camera3D.h"
#include "LGF/Graphics/Graphics3D.h"
#include "LGF/Math/MathUtils.h"

using namespace LGF;

Camera3D::Camera3D() {
}

Camera3D::Camera3D(Vector3 translation, float yaw, float pitch, const Param& param) :
	translation(translation), yaw(yaw), pitch(pitch), param(param) {
}

Camera3D::~Camera3D() = default;

void Camera3D::SetScene() const {
	Graphics3D::SetCamera(*this);
}