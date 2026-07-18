#include "LGF/Graphics/Texture.h"

#include "Graphics/Texture/TextureInternal.h"

using namespace LGF;

Texture::Texture(const std::string& filePath, const TextureLoadDesc& desc) {
	*this = TextureInternal::Load(filePath, desc);
}

Texture::Texture(Handle handle) : handle_(handle) {
}

bool Texture::IsValid() const {
	return TextureInternal::IsValid(*this);
}

uint32_t Texture::Width() const {
	return TextureInternal::GetWidth(*this);
}

uint32_t Texture::Height() const {
	return TextureInternal::GetHeight(*this);
}

bool Texture::IsCubeMap() const {
	return TextureInternal::IsCubeMap(*this);
}
