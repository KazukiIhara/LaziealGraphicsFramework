#include "TriangleDrawer3D.h"

#include <cassert>

#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/Pipeline/Manager/PipelineManager.h"
#include "Graphics/Camera/System/Camera3DSystem.h"
#include "Graphics/DirectX/ViewManagers/SRVUAVManager/SRVUAVManager.h"

#include "Graphics/Pipeline/Pipeline.h"

using namespace LGF;

TriangleDrawer3D::TriangleDrawer3D(Graphics3DContext context) :
	BaseDrawer3D(context) {
}

TriangleDrawer3D::~TriangleDrawer3D() = default;

void TriangleDrawer3D::Initialize() {
	// Drawer初期化
	BaseDrawer::InitParam initParam{
		4096,
		sizeof(Triangle3DForGPU),
		reinterpret_cast<void**>(&instanceData_)
	};
	BaseDrawer::Initialize(initParam);

	// パイプラインマネージャからパイプラインを取得
	SetPipeline(GetContext().pipelineMgr.GetPipeline("Triangle3D"));
	assert(GetPipeline() != nullptr);
}

void TriangleDrawer3D::EndFrame() {
	// インスタンスの数が0なら描画しない
	if (CheckIntstanceCount() == 0) {
		return;
	}

	// コマンドリスト取得
	ID3D12GraphicsCommandList6* commandList = GetContext().directX.GetCommandList6();

	// ルートシグネイチャとパイプラインステート設定
	commandList->SetGraphicsRootSignature(GetPipeline()->GetRootSignature());
	commandList->SetPipelineState(GetPipeline()->GetPipelineState());

	// カメラバインド
	GetContext().cameraSys.Bind(commandList, GetPipeline()->GetRootIndex("gCamera"));

	// インスタンシングリソースバインド
	commandList->SetGraphicsRootDescriptorTable(GetPipeline()->GetRootIndex("gTriangle"), GetContext().directX.GetResMgr().srvuav->GetGPUDescriptorHandle(instanceSrvIndex_));

	// インスタンシング描画実行
	commandList->DispatchMesh(instanceCount_, 1, 1);
}

void TriangleDrawer3D::AddTriangle(const Triangle3DForGPU& data) {
	assert(currentIndex_ < numMaxInstance_);
	instanceData_[currentIndex_] = data;
	++currentIndex_;
}