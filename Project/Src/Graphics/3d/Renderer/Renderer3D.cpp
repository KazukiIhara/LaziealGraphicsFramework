#include "Renderer3D.h"

#include "3d/Drawer/BaseDrawer/BaseDrawer3D.h"
#include "Pipeline/Manager/PipelineManager.h"

#include "3d/Drawer/Primitive/Triangle/TriangleDrawer3D.h"

#include "LGF/Math/MathUtils.h"

using namespace LGF;

Renderer3D::Renderer3D(Graphics3DContext context) :
	context_(context) {
}

Renderer3D::~Renderer3D() = default;

bool Renderer3D::Initialize() {
	// レンダラーを作成
	triangleDrawer3D_ = std::make_unique<TriangleDrawer3D>(context_);
	triangleDrawer3D_->Initialize();
	// 更新用リストに追加
	drawerList_.push_back(triangleDrawer3D_.get());

	return true;
}

void Renderer3D::BeginFrame() {
}

void Renderer3D::EndFrame() {
	for (const auto& drawer : drawerList_) {
		drawer->EndFrame();
	}
}

void Renderer3D::Finalize() {
	triangleDrawer3D_.reset();
}
