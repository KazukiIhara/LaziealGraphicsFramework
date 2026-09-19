#include "Renderer2D.h"

#include "Graphics/Drawer/IDrawer.h"
#include "Graphics/Pipeline/Manager/PipelineManager.h"

#include "Graphics/Drawer/2D/Primitive/Circle/CircleDrawer2D.h"
#include "Graphics/Drawer/2D/Primitive/Rect/RectDrawer2D.h"
#include "Graphics/Drawer/2D/Primitive/RoundRect/RoundRectDrawer2D.h"
#include "Graphics/Drawer/2D/Primitive/Triangle/TriangleDrawer2D.h"
#include "LGF/Graphics/Circle2D.h"
#include "LGF/Graphics/Graphics2D.h"
#include "LGF/Graphics/Line2D.h"
#include "LGF/Graphics/Rect2D.h"
#include "LGF/Graphics/RoundRect2D.h"
#include "LGF/Graphics/Triangle2D.h"

#include <algorithm>
#include <cmath>

using namespace LGF;

namespace {
	// スクリーン座標をクリップ空間座標に変換
	Vector2 ToClipSpace(const Vector2& screenPosition) {
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
	// レンダラー作成
	triangleDrawer_ = std::make_unique<TriangleDrawer2D>(context_);
	triangleDrawer_->Initialize();
	rectDrawer_ = std::make_unique<RectDrawer2D>(context_);
	rectDrawer_->Initialize();
	circleDrawer_ = std::make_unique<CircleDrawer2D>(context_);
	circleDrawer_->Initialize();
	roundRectDrawer_ = std::make_unique<RoundRectDrawer2D>(context_);
	roundRectDrawer_->Initialize();
	// 更新用リストに追加
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

void Renderer2D::AddTriangle(const Triangle2D& triangle, const Vector4& color) {
	Triangle2DForGPU data{};

	data.instance.baseColor = color;

	for (uint32_t i = 0; i < triangle.vertices.size(); ++i) {
		data.vertexOffsets[i] = ToClipSpace(triangle.vertices[i]);
	}

	triangleDrawer_->AddTriangle(data);
}

void Renderer2D::AddRect(const Rect2D& rect, const Vector4& color) {
	Rect2DForGPU data{};

	data.instance.baseColor = color;

	const Vector2 leftTop = rect.leftTop;
	const Vector2 rightTop = { rect.leftTop.x + rect.size.x, rect.leftTop.y };
	const Vector2 leftBottom = { rect.leftTop.x, rect.leftTop.y + rect.size.y };
	const Vector2 rightBottom = { rect.leftTop.x + rect.size.x, rect.leftTop.y + rect.size.y };

	data.vertices[0] = ToClipSpace(leftTop);
	data.vertices[1] = ToClipSpace(rightTop);
	data.vertices[2] = ToClipSpace(leftBottom);
	data.vertices[3] = ToClipSpace(rightBottom);

	rectDrawer_->AddRect(data);
}

void Renderer2D::AddCircle(const Circle2D& circle, const Vector4& color) {
	Circle2DForGPU data{};

	const float width = static_cast<float>(Graphics2D::GetWindowWidth());
	const float height = static_cast<float>(Graphics2D::GetWindowHeight());

	data.instance.baseColor = color;
	data.center = ToClipSpace(circle.center);
	data.radius = {
		(circle.radius / width) * 2.0f,
		(circle.radius / height) * 2.0f
	};

	circleDrawer_->AddCircle(data);
}

void Renderer2D::AddLine(const Line2D& line, const Vector4& color) {
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

	data.instance.baseColor = color;
	data.vertices[0] = ToClipSpace(line.start - normal);
	data.vertices[1] = ToClipSpace(line.start + normal);
	data.vertices[2] = ToClipSpace(line.end - normal);
	data.vertices[3] = ToClipSpace(line.end + normal);

	rectDrawer_->AddRect(data);
}

void Renderer2D::AddRoundRect(const RoundRect2D& roundRect, const Vector4& color) {
	if (roundRect.size.x <= 0.0f || roundRect.size.y <= 0.0f) {
		return;
	}

	const float width = static_cast<float>(Graphics2D::GetWindowWidth());
	const float height = static_cast<float>(Graphics2D::GetWindowHeight());
	const float radius = std::clamp(roundRect.radius, 0.0f, ((roundRect.size.x < roundRect.size.y) ? roundRect.size.x : roundRect.size.y) * 0.5f);

	RoundRect2DForGPU data{};

	data.instance.baseColor = color;
	data.leftTop = ToClipSpace(roundRect.leftTop);
	data.rightBottom = ToClipSpace(roundRect.leftTop + roundRect.size);
	data.radius = {
		(radius / width) * 2.0f,
		(radius / height) * 2.0f
	};

	roundRectDrawer_->AddRoundRect(data);
}

