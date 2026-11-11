#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string_view>

#include "LGF/Graphics/Color.h"
#include "LGF/Math/MathTypes.h"

namespace LGF {

	struct FontResource;
	class Renderer3D;

	enum class Font3DMode : uint8_t {
		Billboard,
		World,
	};

	struct Font3DDrawOptions {
		Vector4 color = Color::White;
		// 0.0が左上、0.5が中央、1.0が右下を表す。
		Vector2 anchor{};
		float worldUnitsPerPixel = 0.01f;
		Font3DMode mode = Font3DMode::Billboard;
		Quaternion rotation{};
	};

	/// <summary>
	/// 3D空間へ描画するフォント
	/// </summary>
	class Font3D {
	public:
		Font3D() = default;
		/// <summary>パスからフォントを同期読み込み。失敗時は例外を送出</summary>
		explicit Font3D(const std::filesystem::path& filePath, uint32_t pixelSize = 64u);
		~Font3D();

		/// <summary>UTF-8文字列のanchor位置を3D空間上の指定座標へ合わせて描画</summary>
		void Draw(
			std::string_view text,
			const Vector3& position,
			const Font3DDrawOptions& options = {}) const;

		/// <summary>UTF-8文字列の領域をグリフのピクセル単位で取得</summary>
		Vector2 Measure(std::string_view text) const;

		const std::filesystem::path& GetFilePath() const;
		uint32_t GetPixelSize() const;
		bool IsValid() const;

	private:
		friend class Renderer3D;

		std::shared_ptr<FontResource> resource_;
		std::filesystem::path filePath_;
		uint32_t pixelSize_{};
	};

}
