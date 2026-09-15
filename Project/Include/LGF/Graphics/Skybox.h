#pragma once

#include <string>

#include "LGF/Graphics/Texture.h"

namespace LGF {

	class SkyboxSystem;

	/// <summary>
	/// 正距円筒図法のHDR環境テクスチャを背景として表示するSkybox
	/// </summary>
	class Skybox {
	public:
		explicit Skybox(const std::string& filePath);
		~Skybox() = default;

		/// <summary>このフレームで使用するSkyboxとして設定</summary>
		void SetScene() const;

		/// <summary>HDR輝度へ乗算する強度。負の値は0として扱う。</summary>
		float intensity = 1.0f;
		/// <summary>Y軸周りの回転角（ラジアン）</summary>
		float rotation = 0.0f;

	private:
		Texture texture_;

		friend class SkyboxSystem;
	};

}
