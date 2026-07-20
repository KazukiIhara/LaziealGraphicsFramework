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
	// テクスチャがすでに読み込まれているかチェックする
	auto it = textures_.find(filePath);
	if (it != textures_.end()) {
		return;
	}

	TextureData& texture = textures_[filePath];
	DirectX::ScratchImage mipImage;



}

void LGF::TextureManager::LoadAll() {
}

const LGF::TextureData& LGF::TextureManager::Find(const std::string& name) {
	return TextureData();
}
