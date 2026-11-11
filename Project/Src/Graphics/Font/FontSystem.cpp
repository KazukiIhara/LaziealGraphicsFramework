#include "FontSystemInternal.h"

#include <cassert>

namespace {
	LGF::FontManager* g_fontManager = nullptr;
}

void LGF::FontSystem::Internal::SetFontManager(FontManager* fontManager) {
	assert(fontManager != nullptr);
	g_fontManager = fontManager;
}

void LGF::FontSystem::Internal::ResetFontManager() {
	g_fontManager = nullptr;
}

LGF::FontManager* LGF::FontSystem::Internal::GetFontManager() {
	return g_fontManager;
}
