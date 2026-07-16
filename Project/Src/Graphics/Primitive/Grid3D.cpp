#include "LGF/Graphics/Grid3D.h"

#include <cmath>

#include "LGF/Graphics/Color.h"
#include "LGF/Graphics/Line3D.h"

using namespace LGF;

Grid3D::Grid3D() {
}

Grid3D::Grid3D(float size, float gridSpacing) :
	size(size),
	gridSpacing(gridSpacing) {
}

Grid3D::~Grid3D() = default;

void Grid3D::Draw(const Matrix4x4& worldMatrix) const {
	if (size <= 0.0f || gridSpacing <= 0.0f) {
		return;
	}

	const float halfSize = size * 0.5f;
	const int halfLineCount = static_cast<int>(std::floor(halfSize / gridSpacing));

	for (int i = -halfLineCount; i <= halfLineCount; ++i) {
		if (i == 0) {
			continue;
		}

		const float offset = static_cast<float>(i) * gridSpacing;

		Line3D({ -halfSize, 0.0f, offset }, { halfSize, 0.0f, offset }).Draw(worldMatrix, Color::Gray);
		Line3D({ offset, 0.0f, -halfSize }, { offset, 0.0f, halfSize }).Draw(worldMatrix, Color::Gray);
	}

	Line3D({ -halfSize, 0.0f, 0.0f }, { halfSize, 0.0f, 0.0f }).Draw(worldMatrix, Color::Red);
	Line3D({ 0.0f, 0.0f, -halfSize }, { 0.0f, 0.0f, halfSize }).Draw(worldMatrix, Color::Blue);
}
