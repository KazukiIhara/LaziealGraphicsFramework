#include "LGF/Graphics/Camera2D.h"
#include "LGF/Graphics/Graphics2D.h"

#include <cstdint>

using namespace LGF;

Camera2D::Camera2D() {
	const uint32_t windowWidth = Graphics2D::GetWindowWidth();
	const uint32_t windowHeight = Graphics2D::GetWindowHeight();
	size = {
		static_cast<float>(windowWidth),
		static_cast<float>(windowHeight)
	};
	translation = {
		size.x * 0.5f,
		size.y * 0.5f
	};
}

Camera2D::Camera2D(Vector2 translation, float rotation, Vector2 size) :
	translation(translation), rotation(rotation), size(size) {
}

Camera2D::~Camera2D() = default;

void Camera2D::Set() const {
}
