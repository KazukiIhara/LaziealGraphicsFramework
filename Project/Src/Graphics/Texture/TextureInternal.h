#pragma once

#include <string>

#include "LGF/Graphics/Texture.h"

namespace LGF {
	class TextureManager;

	namespace TextureInternal {
		void SetManager(TextureManager* manager);
		void ResetManager();
		Texture Load(const std::string& filePath, const TextureLoadDesc& desc);
		bool IsValid(const Texture& texture);
		uint32_t GetWidth(const Texture& texture);
		uint32_t GetHeight(const Texture& texture);
		bool IsCubeMap(const Texture& texture);
	}
}
