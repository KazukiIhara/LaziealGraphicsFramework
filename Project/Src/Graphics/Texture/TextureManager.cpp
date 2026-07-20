#include "TextureManager.h"


LGF::TextureManager::TextureManager() {
}

LGF::TextureManager::~TextureManager() = default;

bool LGF::TextureManager::Initialize() {

	return true;
}

void LGF::TextureManager::Finalize() {
}

void LGF::TextureManager::Load(const std::string& filePath, const TextureLoadDesc& desc) {
}

void LGF::TextureManager::LoadAll() {
}

const LGF::TextureData& LGF::TextureManager::Find(const std::string& name) {
	return TextureData();
}
