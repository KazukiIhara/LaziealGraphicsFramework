#include "RenderTextureManager.h"

#include <cassert>

using namespace LGF;

RenderTextureManager::RenderTextureManager(
	ResourceFactory& resourceFactory,
	RTVManager& rtvManager,
	SRVUAVManager& srvUavManager)
	: resourceFactory_(resourceFactory),
	rtvManager_(rtvManager),
	srvUavManager_(srvUavManager) {
}

RenderTextureManager::~RenderTextureManager() = default;

RenderTexture* RenderTextureManager::Create(const std::string& name, const RenderTextureDesc& desc) {
	assert(!name.empty());

	auto renderTexture = std::make_unique<RenderTexture>(
		resourceFactory_,
		rtvManager_,
		srvUavManager_);

	if (!renderTexture->Initialize(desc)) {
		return nullptr;
	}

	RenderTexture* result = renderTexture.get();
	renderTextures_[name] = std::move(renderTexture);
	return result;
}

RenderTexture* RenderTextureManager::Get(const std::string& name) {
	auto it = renderTextures_.find(name);
	if (it == renderTextures_.end()) {
		return nullptr;
	}

	return it->second.get();
}

const RenderTexture* RenderTextureManager::Get(const std::string& name) const {
	auto it = renderTextures_.find(name);
	if (it == renderTextures_.end()) {
		return nullptr;
	}

	return it->second.get();
}

bool RenderTextureManager::ResizeAll(uint32_t width, uint32_t height) {
	for (auto& [name, renderTexture] : renderTextures_) {
		(void)name;
		if (!renderTexture->Resize(width, height)) {
			return false;
		}
	}

	return true;
}

void RenderTextureManager::Finalize() {
	renderTextures_.clear();
}
