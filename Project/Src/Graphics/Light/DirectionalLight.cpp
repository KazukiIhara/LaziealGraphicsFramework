#include "LGF/Graphics/DirectionalLight.h"

#include "LGF/Graphics/Graphics3D.h"

using namespace LGF;

DirectionalLight::DirectionalLight(const Vector3& direction, const Vector4& color, float intensity)
	: direction(direction), color(color), intensity(intensity) {
}

void DirectionalLight::SetScene() const {
	Graphics3D::SetDirectionalLight(*this);
}
