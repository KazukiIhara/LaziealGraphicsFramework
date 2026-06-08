#include "Renderer3D.h"

#include "3d/BaseDrawer/BaseDrawer3D.h"
#include "Pipeline/Manager/PipelineManager.h"

using namespace LGF;

Renderer3D::Renderer3D(PipelineManager* pipelineMgr) :
	pipelineMgr_(pipelineMgr) {
}

Renderer3D::~Renderer3D() = default;

void Renderer3D::Initialize() {
	// レンダラーを作成

	// パイプラインを生成してレンダラーにセット

	// 更新用リストに追加

}

void Renderer3D::Render() {

}
