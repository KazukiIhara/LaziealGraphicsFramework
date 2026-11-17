#include "LGF/Graphics/ToonLighting.h"

#include "LGF/Graphics/Graphics3D.h"

using namespace LGF;

void ToonLighting::SetScene() const {
	Graphics3D::SetToonLighting(*this);
}
