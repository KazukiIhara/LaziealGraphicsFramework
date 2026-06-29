#include "TriangleDrawer2D.h"

#include <cassert>


#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/Pipeline/Manager/PipelineManager.h"
#include "Graphics/Camera/System/Camera3DSystem.h"
#include "Graphics/DirectX/ViewManagers/SRVUAVManager/SRVUAVManager.h"

#include "Graphics/Pipeline/Pipeline.h"

using namespace LGF;

TriangleDrawer2D::TriangleDrawer2D(Graphics2DContext context) :
	BaseDrawer2D(context) {
}

TriangleDrawer2D::~TriangleDrawer2D() = default;

void TriangleDrawer2D::Initialize() {
	// Drawer初期化
	BaseDrawer::InitParam initParam{
		4096,
		sizeof(Triangle2DForGPU),
		reinterpret_cast<void**>(&instanceData_)
	};
	BaseDrawer::Initialize(initParam);

	// パイプラインマネージャからパイプラインを取得

}

void TriangleDrawer2D::EndFrame() {
	// インスタンスの数が0なら描画しない
	if (CheckIntstanceCount() == 0) {
		return;
	}

}

void TriangleDrawer2D::AddTriangle(const Triangle2DForGPU& data) {
	assert(currentIndex_ < numMaxInstance_);
	instanceData_[currentIndex_] = data;
	++currentIndex_;
}
