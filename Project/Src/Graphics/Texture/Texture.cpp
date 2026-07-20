#include "LGF/Graphics/Texture.h"

#include <cassert>

#include "TextureSystemInternal.h"
#include "TextureManager.h"

namespace {
	LGF::TextureManager* g_textureManager = nullptr;
}

void LGF::TextureSystem::Internal::SetTextureManager(TextureManager* textureManager) {
	assert(textureManager != nullptr);
	g_textureManager = textureManager;
}

void LGF::TextureSystem::Internal::ResetTextureManager() {
	g_textureManager = nullptr;
}

LGF::Texture::Texture(const std::string& name, const TextureLoadDesc& desc) {
	assert(g_textureManager != nullptr);
	g_textureManager->Load(name, desc);
	filePath_ = name;
}

LGF::Texture::~Texture() = default;

const std::string& LGF::Texture::GetFilePath() const {
	return filePath_;
}

LGF::TextureLoadDesc LGF::TextureLoadDesc::Color() {
	return {
	   .dimension = TextureDimension::Texture2D,
	   .colorSpace = TextureColorSpace::SRGB,
	   .generateMipMaps = true,
	};
}

LGF::TextureLoadDesc LGF::TextureLoadDesc::NormalMap() {
	return {
	   .dimension = TextureDimension::Texture2D,
	   .colorSpace = TextureColorSpace::Linear,
	   .generateMipMaps = true,
	};
}

LGF::TextureLoadDesc LGF::TextureLoadDesc::Cubemap() {
	return {
		.dimension = TextureDimension::TextureCube,
		.colorSpace = TextureColorSpace::SRGB,
		.generateMipMaps = true,
	};
}
