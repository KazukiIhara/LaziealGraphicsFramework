#include "LGF/Graphics/Skybox.h"

#include "LGF/Graphics/Graphics3D.h"

using namespace LGF;

Skybox::Skybox(const std::string& filePath)
	: texture_(filePath, TextureLoadDesc{
		.dimension = TextureDimension::Texture2D,
		.colorSpace = TextureColorSpace::Linear,
		.generateMipMaps = true,
	}) {
}

void Skybox::SetScene() const {
	Graphics3D::SetSkybox(*this);
}
