#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string_view>

#include "LGF/Graphics/Color.h"
#include "LGF/Math/Vector2.h"

namespace LGF {

	struct FontResource;
	class Renderer2D;

	struct Font2DDrawOptions {
		Vector4 color = Color::White;
		// 0.0が左上、0.5が中央、1.0が右下を表す。
		Vector2 anchor{};
	};

	/// <summary>
	/// 2D空間へ描画するフォント
	/// </summary>
	class Font2D {
	public:
		Font2D() = default;
		/// <summary>パスからフォントを同期読み込み。失敗時は例外を送出</summary>
		explicit Font2D(const std::filesystem::path& filePath, uint32_t pixelSize = 24u);
		~Font2D();

		/// <summary>UTF-8文字列のanchor位置を指定座標へ合わせて描画</summary>
		void Draw(
			std::string_view text,
			const Vector2& position,
			const Font2DDrawOptions& options = {}) const;

		/// <summary>UTF-8文字列を描画したときの領域をピクセル単位で取得</summary>
		Vector2 Measure(std::string_view text) const;

		const std::filesystem::path& GetFilePath() const;
		uint32_t GetPixelSize() const;
		bool IsValid() const;

	private:
		friend class Renderer2D;

		std::shared_ptr<FontResource> resource_;
		std::filesystem::path filePath_;
		uint32_t pixelSize_{};
	};

}
