#include "LGF/Graphics/Font3D.h"

#include <cmath>
#include <stdexcept>

#include "FontManager.h"
#include "FontSystemInternal.h"
#include "FontUtility.h"
#include "Graphics/Graphics3DInternal.h"

LGF::Font3D::Font3D(const std::filesystem::path& filePath, uint32_t pixelSize) :
	filePath_(filePath),
	pixelSize_(pixelSize) {

	FontManager* fontManager = FontSystem::Internal::GetFontManager();
	if (!fontManager) {
		throw std::runtime_error("Font3D must be loaded after framework initialization.");
	}
	resource_ = fontManager->Load(filePath, pixelSize);
}

LGF::Font3D::~Font3D() = default;

void LGF::Font3D::Draw(
	std::string_view text,
	const Vector3& position,
	const Font3DDrawOptions& options) const {

	if (!resource_) {
		throw std::runtime_error("Cannot draw an empty Font3D.");
	}
	if (!std::isfinite(options.worldUnitsPerPixel) || options.worldUnitsPerPixel <= 0.0f) {
		throw std::invalid_argument("Font3D worldUnitsPerPixel must be greater than zero.");
	}
	if (!FontUtility::IsValidAnchor(options.anchor)) {
		throw std::invalid_argument("Font3D anchor must be between zero and one.");
	}
	FontManager* fontManager = FontSystem::Internal::GetFontManager();
	if (!fontManager) {
		throw std::runtime_error("Font3D cannot be used after framework finalization.");
	}
	fontManager->PrepareText(resource_, text);
	Graphics3D::Internal::DrawFont(*this, text, position, options);
}

LGF::Vector2 LGF::Font3D::Measure(std::string_view text) const {
	if (!resource_) {
		throw std::runtime_error("Cannot measure text with an empty Font3D.");
	}
	FontManager* fontManager = FontSystem::Internal::GetFontManager();
	if (!fontManager) {
		throw std::runtime_error("Font3D cannot be used after framework finalization.");
	}
	return fontManager->Measure(resource_, text);
}

const std::filesystem::path& LGF::Font3D::GetFilePath() const {
	return filePath_;
}

uint32_t LGF::Font3D::GetPixelSize() const {
	return pixelSize_;
}

bool LGF::Font3D::IsValid() const {
	return resource_ != nullptr;
}
