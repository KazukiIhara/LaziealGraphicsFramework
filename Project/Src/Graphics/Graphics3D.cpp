#include "LGF/Graphics/Graphics3D.h"

#include <cassert>

#include "Graphics3DInternal.h"
#include "Graphics/Renderer/Renderer3D.h"

namespace {

	LGF::Renderer3D* g_renderer = nullptr;

}

void LGF::Graphics3D::Internal::SetRenderer(Renderer3D* renderer) {
	assert(renderer != nullptr);
	g_renderer = renderer;
}

void LGF::Graphics3D::Internal::ResetRenderer() {
	g_renderer = nullptr;
}

void LGF::Graphics3D::Draw(const Triangle3D& triangle, const Matrix4x4& worldMatrix, const Vector4& color) {
	assert(g_renderer != nullptr);

	if (g_renderer == nullptr) {
		return;
	}

	g_renderer->AddTriangle(
		triangle,
		worldMatrix,
		color
	);
}
