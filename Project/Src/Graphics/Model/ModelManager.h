#pragma once

#include <memory>
#include <string>
#include <unordered_map>

namespace LGF {

	class DirectXCore;
	class TextureManager;
	struct ModelResource;

	class ModelManager {
	public:
		ModelManager(DirectXCore& directX, TextureManager& textureManager);
		~ModelManager();

		bool Initialize();
		void Finalize();

		std::shared_ptr<ModelResource> Load(const std::string& filePath);

	private:
		DirectXCore& directX_;
		TextureManager& textureManager_;
		std::unordered_map<std::wstring, std::shared_ptr<ModelResource>> models_;
	};

}
