#include "LGF/Graphics/Font2D.h"

#include <stdexcept>

#include "FontManager.h"
#include "FontSystemInternal.h"
#include "FontUtility.h"
#include "Graphics/Graphics2DInternal.h"

LGF::Font2D::Font2D(const std::filesystem::path& filePath, uint32_t pixelSize) :
	filePath_(filePath),
	pixelSize_(pixelSize) {

	FontManager* fontManager = FontSystem::Internal::GetFontManager();
	if (!fontManager) {
		throw std::runtime_error("Font2D must be loaded after framework initialization.");
	}
	resource_ = fontManager->Load(filePath, pixelSize);
}

LGF::Font2D::~Font2D() = default;

void LGF::Font2D::Draw(
	std::string_view text,
	const Vector2& position,
	const Font2DDrawOptions& options) const {

	if (!resource_) {
		throw std::runtime_error("Cannot draw an empty Font2D.");
	}
	if (!FontUtility::IsValidAnchor(options.anchor)) {
		throw std::invalid_argument("Font2D anchor must be between zero and one.");
	}
	FontManager* fontManager = FontSystem::Internal::GetFontManager();
	if (!fontManager) {
		throw std::runtime_error("Font2D cannot be used after framework finalization.");
	}
	fontManager->PrepareText(resource_, text);
	Graphics2D::Internal::DrawFont(*this, text, position, options);
}

LGF::Vector2 LGF::Font2D::Measure(std::string_view text) const {
	if (!resource_) {
		throw std::runtime_error("Cannot measure text with an empty Font2D.");
	}
	FontManager* fontManager = FontSystem::Internal::GetFontManager();
	if (!fontManager) {
		throw std::runtime_error("Font2D cannot be used after framework finalization.");
	}
	return fontManager->Measure(resource_, text);
}

const std::filesystem::path& LGF::Font2D::GetFilePath() const {
	return filePath_;
}

uint32_t LGF::Font2D::GetPixelSize() const {
	return pixelSize_;
}

bool LGF::Font2D::IsValid() const {
	return resource_ != nullptr;
}
