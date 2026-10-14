#include "LGF/Graphics/PointLight.h"

#include "LGF/Graphics/Graphics3D.h"

using namespace LGF;

PointLight::PointLight(
	const Vector3& position,
	const Vector4& color,
	float intensity,
	float range)
	: position(position), color(color), intensity(intensity), range(range) {
}

void PointLight::SetScene() const {
	Graphics3D::SetPointLight(*this);
}
