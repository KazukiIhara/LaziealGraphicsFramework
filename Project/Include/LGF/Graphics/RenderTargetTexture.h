#pragma once

#include <cstdint>
#include <memory>

#include "LGF/Graphics/Color.h"
#include "LGF/Graphics/Texture.h"

namespace LGF {

	struct RenderPassTargetResource;
	class Renderer3D;

	struct RenderTargetTextureDesc {
		uint32_t width = 0u;
		uint32_t height = 0u;
		Vector4 clearColor = Color::Black;
	};

	/// <summary>
	/// 3D描画結果をテクスチャとして保持するレンダーターゲット
	/// </summary>
	class RenderTargetTexture {
	public:
		RenderTargetTexture() = default;
		explicit RenderTargetTexture(const RenderTargetTextureDesc& desc);
		~RenderTargetTexture();

		const Texture& GetTexture() const;
		uint32_t GetWidth() const;
		uint32_t GetHeight() const;
		bool IsValid() const;

	private:
		friend class Renderer3D;
		std::shared_ptr<RenderPassTargetResource> resource_;
		Texture texture_;
	};

}
