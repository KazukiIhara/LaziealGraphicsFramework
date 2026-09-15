#include "LGF/Graphics/Texture.h"

#include <cassert>
#include <stdexcept>

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
	if (!g_textureManager) {
		throw std::runtime_error("Texture must be loaded after framework initialization.");
	}
	const auto& data = g_textureManager->Load(name, desc);
	srvIndex_ = data.srvIndex;
	isCubeMap_ = data.metaData.IsCubemap();
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

LGF::TextureLoadDesc LGF::TextureLoadDesc::Data() {
	return {
	   .dimension = TextureDimension::Texture2D,
	   .colorSpace = TextureColorSpace::Linear,
	   .generateMipMaps = true,
	};
}

LGF::TextureLoadDesc LGF::TextureLoadDesc::NormalMap() {
	return Data();
}

LGF::TextureLoadDesc LGF::TextureLoadDesc::Cubemap() {
	return {
		.dimension = TextureDimension::TextureCube,
		.colorSpace = TextureColorSpace::SRGB,
		.generateMipMaps = true,
	};
}
