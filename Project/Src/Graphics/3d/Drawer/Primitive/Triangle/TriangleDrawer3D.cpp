#include "TriangleDrawer3D.h"

#include "Pipeline/Manager/PipelineManager.h"

using namespace LGF;

TriangleDrawer3D::TriangleDrawer3D(Graphics3DContext context) :
	BaseDrawer3D(context) {
}

TriangleDrawer3D::~TriangleDrawer3D() = default;

void TriangleDrawer3D::Initialize() {
	// Drawer初期化
	BaseDrawer3D::InitParam initParam{
		4098,
		sizeof(Triangle3DForGPU) * 4098,
		reinterpret_cast<void**>(&instData_)
	};
	BaseDrawer3D::Initialize(initParam);

	// パイプラインマネージャからパイプラインを取得
	SetPipeline(GetContext().pipelineMgr.GetPipeline("Triangle3D"));
}

void TriangleDrawer3D::EndFrame() {

}

void TriangleDrawer3D::AddTriangle(const Triangle3DForGPU& data) {
	data;
}