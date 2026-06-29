#include "Renderer2D.h"

#include "Graphics/Drawer/IDrawer.h"
#include "Graphics/Pipeline/Manager/PipelineManager.h"

#include "Graphics/Drawer/2D/Primitive/Triangle/TriangleDrawer2D.h"
#include "LGF/Graphics/Triangle2D.h"

#include "LGF/Math/MathUtils.h"

using namespace LGF;

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

	return false;
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

void Renderer2D::AddTriangle(const Triangle2D& triangle, const Matrix3x3& worldMatrix, const Vector4& color) {
	Triangle2DForGPU data{};

	data.instance.worldMatrix = worldMatrix;

	data.instance.baseColor = color;

	std::ranges::copy(
		triangle.vertices,
		data.vertexOffsets
	);

	triangleDrawer_->AddTriangle(data);
}

