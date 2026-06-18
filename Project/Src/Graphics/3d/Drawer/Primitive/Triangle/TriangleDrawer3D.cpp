#include "TriangleDrawer3D.h"

using namespace LGF;

TriangleDrawer3D::TriangleDrawer3D(Graphics3DContext context) :
	BaseDrawer3D(context) {
}

TriangleDrawer3D::~TriangleDrawer3D() = default;

void TriangleDrawer3D::Initialize() {
	// Drawer初期化
	BaseDrawer3D::InitParam initParam{};
	BaseDrawer3D::Initialize(initParam);

	// パイプラインマネージャからパイプラインを取得

}

void TriangleDrawer3D::EndFrame() {

}

void TriangleDrawer3D::AddTriangle(const Triangle3DForGPU& data) {

}