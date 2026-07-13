#include "CircleDrawer2D.h"

#include <cassert>

#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/Pipeline/Manager/PipelineManager.h"
#include "Graphics/DirectX/ViewManagers/SRVUAVManager/SRVUAVManager.h"

#include "Graphics/Pipeline/Pipeline.h"

using namespace LGF;

CircleDrawer2D::CircleDrawer2D(Graphics2DContext context) :
	BaseDrawer2D(context) {
}

CircleDrawer2D::~CircleDrawer2D() = default;

void CircleDrawer2D::Initialize() {
	// Drawer初期化
	BaseDrawer::InitParam initParam{
		4096,
		sizeof(Circle2DForGPU),
		reinterpret_cast<void**>(&instanceData_)
	};
	BaseDrawer::Initialize(initParam);

	// パイプラインマネージャからパイプラインを取得
	SetPipeline(GetContext().pipelineMgr.GetPipeline("Circle2D"));
	assert(GetPipeline() != nullptr);
}

void CircleDrawer2D::EndFrame() {
	// インスタンスの数がなければ描画しない
	if (CheckIntstanceCount() == 0) {
		return;
	}

	// コマンドリスト取得
	ID3D12GraphicsCommandList6* commandList = GetContext().directX.GetCommandList6();

	// ルートシグネチャとパイプラインステート設定
	commandList->SetGraphicsRootSignature(GetPipeline()->GetRootSignature());
	commandList->SetPipelineState(GetPipeline()->GetPipelineState());

	// インスタンシングリソースバインド
	commandList->SetGraphicsRootDescriptorTable(GetPipeline()->GetRootIndex("gCircle"), GetContext().directX.GetResMgr().srvuav->GetGPUDescriptorHandle(instanceSrvIndex_));

	// インスタンシング描画実行
	commandList->DispatchMesh(instanceCount_, 1, 1);
}

void CircleDrawer2D::AddCircle(const Circle2DForGPU& data) {
	assert(currentIndex_ < numMaxInstance_);
	instanceData_[currentIndex_] = data;
	++currentIndex_;
}
