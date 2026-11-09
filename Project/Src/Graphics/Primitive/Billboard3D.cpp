#include "LGF/Graphics/Billboard3D.h"

#include "LGF/Graphics/Graphics3D.h"

void LGF::Billboard3D::Draw(const Vector4& color, const Texture& texture) const {
	Graphics3D::DrawBillboard(*this, color, texture);
}
