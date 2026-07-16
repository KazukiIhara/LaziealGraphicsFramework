#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "Graphics/RenderTexture/RenderTexture.h"

namespace LGF {

	class ResourceFactory;
	class RTVManager;
	class SRVUAVManager;

	/// <summary>
	/// 名前付きRenderTextureを保持し、メインViewportに合わせてResizeする管理クラス
	/// </summary>
	class RenderTextureManager {
	public:
		RenderTextureManager(
			ResourceFactory& resourceFactory,
			RTVManager& rtvManager,
			SRVUAVManager& srvUavManager);
		~RenderTextureManager();

		RenderTexture* Create(const std::string& name, const RenderTextureDesc& desc);
		RenderTexture* Get(const std::string& name);
		const RenderTexture* Get(const std::string& name) const;

		bool ResizeAll(uint32_t width, uint32_t height);
		void Finalize();

	private:
		ResourceFactory& resourceFactory_;
		RTVManager& rtvManager_;
		SRVUAVManager& srvUavManager_;

		std::unordered_map<std::string, std::unique_ptr<RenderTexture>> renderTextures_;
	};
}
