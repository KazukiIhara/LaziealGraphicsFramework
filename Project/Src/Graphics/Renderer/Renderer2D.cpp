#include "Renderer2D.h"

#include "Graphics/Drawer/IDrawer.h"
#include "Graphics/Pipeline/Manager/PipelineManager.h"

#include "Graphics/Drawer/2D/Primitive/Triangle/TriangleDrawer2D.h"
#include "LGF/Graphics/Graphics2D.h"
#include "LGF/Graphics/Triangle2D.h"

using namespace LGF;

namespace {

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
	// 更新用リストに追加
	drawerList_.push_back(triangleDrawer_.get());

	return true;
}

void Renderer2D::BeginFrame() {
}

void Renderer2D::EndFrame() {
	for (const auto& drawer : drawerList_) {
		drawer->EndFrame();
	}
}

void Renderer2D::Finalize() {
	triangleDrawer_.reset();
}

void Renderer2D::AddTriangle(const Triangle2D& triangle, const Vector4& color) {
	Triangle2DForGPU data{};

	data.instance.baseColor = color;

	for (uint32_t i = 0; i < triangle.vertices.size(); ++i) {
		data.vertexOffsets[i] = ToClipSpace(triangle.vertices[i]);
	}

	triangleDrawer_->AddTriangle(data);
}

