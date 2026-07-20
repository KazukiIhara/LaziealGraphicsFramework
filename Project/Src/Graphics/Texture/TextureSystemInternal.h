#pragma once


namespace LGF {

	class TextureManager;

	namespace TextureSystem::Internal {

		void SetTextureManager(TextureManager* textureManager);
		void ResetTextureManager();

	}

}