#include "LGF/Graphics/RenderTargetTexture.h"

#include <stdexcept>

#include "Graphics/Graphics3DInternal.h"
#include "Graphics/RenderPass/RenderPassTargetResource.h"

using namespace LGF;

RenderTargetTexture::RenderTargetTexture(const RenderTargetTextureDesc& desc) {
	resource_ = Graphics3D::Internal::CreateRenderTarget(desc, texture_);
	if (!resource_) {
		throw std::runtime_error("RenderTargetTexture creation failed.");
	}
}

RenderTargetTexture::~RenderTargetTexture() = default;

const Texture& RenderTargetTexture::GetTexture() const {
	return texture_;
}

uint32_t RenderTargetTexture::GetWidth() const {
	return resource_ ? resource_->width : 0u;
}

uint32_t RenderTargetTexture::GetHeight() const {
	return resource_ ? resource_->height : 0u;
}

bool RenderTargetTexture::IsValid() const {
	return resource_ && texture_.IsValid();
}
