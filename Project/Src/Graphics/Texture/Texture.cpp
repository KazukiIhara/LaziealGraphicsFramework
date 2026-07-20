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
	const DirectX::TexMetadata metaData =
		g_textureManager->Find(name).metaData;

	width_ = metaData.width;
	height_ = metaData.height;
	isCubemap_ = metaData.IsCubemap();

	srvIndex_ = g_textureManager->Find(name).srvIndex;

	filePath_ = name;
}

LGF::Texture::~Texture() = default;

const std::string& LGF::Texture::GetFilePath() const {
	return filePath_;
}

uint32_t LGF::Texture::GetWidth()const {
	return width_;
}

uint32_t LGF::Texture::GetHeight()const {
	return height_;
}

uint32_t LGF::Texture::GetSrvIndex() const {
	return srvIndex_;
}
