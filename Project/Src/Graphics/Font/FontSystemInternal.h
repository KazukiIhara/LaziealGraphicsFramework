#pragma once

namespace LGF {

	class FontManager;

	namespace FontSystem::Internal {
		void SetFontManager(FontManager* fontManager);
		void ResetFontManager();
		FontManager* GetFontManager();
	}

}
