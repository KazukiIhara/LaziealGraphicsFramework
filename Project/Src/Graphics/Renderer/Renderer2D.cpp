#include "Renderer2D.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "Graphics/Drawer/IDrawer.h"
#include "Graphics/Drawer/2D/Primitive/Circle/CircleDrawer2D.h"
#include "Graphics/Drawer/2D/Primitive/Rect/RectDrawer2D.h"
#include "Graphics/Drawer/2D/Primitive/RoundRect/RoundRectDrawer2D.h"
#include "Graphics/Drawer/2D/Primitive/Triangle/TriangleDrawer2D.h"
#include "Graphics/Font/FontResource.h"
#include "Graphics/Font/FontLayout.h"
#include "LGF/Graphics/Circle2D.h"
#include "LGF/Graphics/Graphics2D.h"
#include "LGF/Graphics/Font2D.h"
#include "LGF/Graphics/Line2D.h"
#include "LGF/Graphics/Material.h"
#include "LGF/Graphics/Rect2D.h"
#include "LGF/Graphics/RoundRect2D.h"
#include "LGF/Graphics/Triangle2D.h"

using namespace LGF;

namespace {

	Vector2 ScreenToClipSpace(const Vector2& screenPosition) {
		const float width = static_cast<float>(Graphics2D::GetWindowWidth());
		const float height = static_cast<float>(Graphics2D::GetWindowHeight());

		return {
			(screenPosition.x / width) * 2.0f - 1.0f,
			1.0f - (screenPosition.y / height) * 2.0f
		};
	}

}

Renderer2D::Renderer2D(Graphics2DContext context) :
	context_(context) {
}

Renderer2D::~Renderer2D() = default;

bool Renderer2D::Initialize() {
	// 描画クラスを作成
	triangleDrawer_ = std::make_unique<TriangleDrawer2D>(context_);
	triangleDrawer_->Initialize();
	rectDrawer_ = std::make_unique<RectDrawer2D>(context_);
	rectDrawer_->Initialize();
	circleDrawer_ = std::make_unique<CircleDrawer2D>(context_);
	circleDrawer_->Initialize();
	roundRectDrawer_ = std::make_unique<RoundRectDrawer2D>(context_);
	roundRectDrawer_->Initialize();

	// フレーム処理の対象を登録
	drawerList_.push_back(triangleDrawer_.get());
	drawerList_.push_back(rectDrawer_.get());
	drawerList_.push_back(circleDrawer_.get());
	drawerList_.push_back(roundRectDrawer_.get());

	return true;
}

void Renderer2D::BeginFrame() {
	for (const auto& drawer : drawerList_) {
		drawer->BeginFrame();
	}
}

void Renderer2D::EndFrame() {
	for (const auto& drawer : drawerList_) {
		drawer->EndFrame();
	}
}

void Renderer2D::Finalize() {
	triangleDrawer_.reset();
	rectDrawer_.reset();
	circleDrawer_.reset();
	roundRectDrawer_.reset();
}

void Renderer2D::AddTriangle(const Triangle2D& triangle, const Material2D& material) {
	Triangle2DForGPU data{};

	ApplyMaterial(data.instance, material);

	for (uint32_t i = 0; i < triangle.vertices.size(); ++i) {
		data.vertexOffsets[i] = ScreenToClipSpace(triangle.vertices[i]);
	}

	triangleDrawer_->AddTriangle(data);
}

void Renderer2D::AddRect(const Rect2D& rect, const Material2D& material) {
	Rect2DForGPU data{};

	ApplyMaterial(data.instance, material);

	const Vector2 leftTop = rect.leftTop;
	const Vector2 rightTop = { rect.leftTop.x + rect.size.x, rect.leftTop.y };
	const Vector2 leftBottom = { rect.leftTop.x, rect.leftTop.y + rect.size.y };
	const Vector2 rightBottom = { rect.leftTop.x + rect.size.x, rect.leftTop.y + rect.size.y };

	data.vertices[0] = ScreenToClipSpace(leftTop);
	data.vertices[1] = ScreenToClipSpace(rightTop);
	data.vertices[2] = ScreenToClipSpace(leftBottom);
	data.vertices[3] = ScreenToClipSpace(rightBottom);

	rectDrawer_->AddRect(data);
}

void Renderer2D::AddCircle(const Circle2D& circle, const Material2D& material) {
	Circle2DForGPU data{};

	const float width = static_cast<float>(Graphics2D::GetWindowWidth());
	const float height = static_cast<float>(Graphics2D::GetWindowHeight());

	ApplyMaterial(data.instance, material);
	data.center = ScreenToClipSpace(circle.center);
	data.radius = {
		(circle.radius / width) * 2.0f,
		(circle.radius / height) * 2.0f
	};

	circleDrawer_->AddCircle(data);
}

void Renderer2D::AddLine(const Line2D& line, const Material2D& material) {
	if (line.thickness <= 0.0f) {
		return;
	}

	const Vector2 direction = line.end - line.start;
	const float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
	if (length <= 0.0f) {
		return;
	}

	const float halfThickness = line.thickness * 0.5f;
	const Vector2 normal = {
		-direction.y / length * halfThickness,
		direction.x / length * halfThickness
	};

	Rect2DForGPU data{};

	ApplyMaterial(data.instance, material);
	data.vertices[0] = ScreenToClipSpace(line.start - normal);
	data.vertices[1] = ScreenToClipSpace(line.start + normal);
	data.vertices[2] = ScreenToClipSpace(line.end - normal);
	data.vertices[3] = ScreenToClipSpace(line.end + normal);

	rectDrawer_->AddRect(data);
}

void Renderer2D::AddRoundRect(const RoundRect2D& roundRect, const Material2D& material) {
	if (roundRect.size.x <= 0.0f || roundRect.size.y <= 0.0f) {
		return;
	}

	const float width = static_cast<float>(Graphics2D::GetWindowWidth());
	const float height = static_cast<float>(Graphics2D::GetWindowHeight());
	const float maximumRadius = std::min(roundRect.size.x, roundRect.size.y) * 0.5f;
	const float radius = std::clamp(roundRect.radius, 0.0f, maximumRadius);

	RoundRect2DForGPU data{};

	ApplyMaterial(data.instance, material);
	data.leftTop = ScreenToClipSpace(roundRect.leftTop);
	data.rightBottom = ScreenToClipSpace(roundRect.leftTop + roundRect.size);
	data.radius = {
		(radius / width) * 2.0f,
		(radius / height) * 2.0f
	};

	roundRectDrawer_->AddRoundRect(data);
}

void Renderer2D::AddText(
	const Font2D& font,
	std::string_view text,
	const Vector2& position,
	const Font2DDrawOptions& options) {

	if (!font.resource_ || font.resource_->atlasSrvIndex == UINT32_MAX) {
		throw std::invalid_argument("Font resource is not ready for drawing.");
	}

	const FontResource& resource = *font.resource_;
	const FontTextLayout layout = BuildFontTextLayout(resource, text);
	const Vector2 anchorOffset = layout.size * options.anchor;
	for (const FontGlyphLayout& glyph : layout.glyphs) {
		const Vector2 glyphLeftTop = position + glyph.leftTop - anchorOffset;
		Rect2DForGPU data{};
		data.instance.baseColor = options.color;
		data.instance.textureIndex = resource.atlasSrvIndex;
		data.instance.uvScale = glyph.uvScale;
		data.instance.uvOffset = glyph.uvOffset;
		data.vertices[0] = ScreenToClipSpace(glyphLeftTop);
		data.vertices[1] = ScreenToClipSpace({ glyphLeftTop.x + glyph.size.x, glyphLeftTop.y });
		data.vertices[2] = ScreenToClipSpace({ glyphLeftTop.x, glyphLeftTop.y + glyph.size.y });
		data.vertices[3] = ScreenToClipSpace(glyphLeftTop + glyph.size);
		rectDrawer_->AddRect(data);
	}
}

void Renderer2D::ApplyMaterial(InstanceData2D& instance, const Material2D& material) {
	if (material.baseColorTexture.isCubeMap_) {
		throw std::invalid_argument("Material2D baseColorTexture must be a Texture2D.");
	}

	instance.baseColor = material.color;
	instance.textureIndex = material.baseColorTexture.srvIndex_;
	instance.uvScale = material.uvScale;
	instance.uvOffset = material.uvOffset;
}
