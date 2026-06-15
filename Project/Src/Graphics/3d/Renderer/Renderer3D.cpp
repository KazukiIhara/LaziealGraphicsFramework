#include "Renderer3D.h"

#include "3d/Drawer/BaseDrawer/BaseDrawer3D.h"
#include "Pipeline/Manager/PipelineManager.h"

#include "3d/Drawer/Primitive/Triangle/TriangleDrawer3D.h"

using namespace LGF;

Renderer3D::Renderer3D(PipelineManager& pipelineMgr) :
	pipelineMgr_(pipelineMgr) {
}

Renderer3D::~Renderer3D() = default;

bool Renderer3D::Initialize() {
	// レンダラーを作成

	// パイプラインをレンダラーにセット

	// 更新用リストに追加


	return true;
}

void Renderer3D::BeginFrame() {

}

void Renderer3D::EndFrame() {

}

void Renderer3D::Finalize() {
	triangleDrawer3D_.reset();
}
