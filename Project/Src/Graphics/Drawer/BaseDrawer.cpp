#include "BaseDrawer.h"

#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/DirectX/ResourceFactory/ResourceFactory.h"
#include "Graphics/DirectX/ViewManagers/SRVUAVManager/SRVUAVManager.h"

using namespace LGF;

BaseDrawer::BaseDrawer(DirectXCore& directX, PipelineManager& pipelineMgr) :
	directX_(directX), pipelineMgr_(pipelineMgr) {
}

BaseDrawer::~BaseDrawer() = default;

Pipeline* BaseDrawer::GetPipeline() {
	return pipeline_;
}

void BaseDrawer::SetPipeline(Pipeline* pipeline) {
	pipeline_ = pipeline;
}

void BaseDrawer::Initialize(InitParam param) {
	// インスタンスの最大数を取得
	numMaxInstance_ = param.numMaxInstance;

	// インスタンスリソース作成
	CreateInstanceResource(param.instResSize, param.instData, instanceResource_, instanceSrvIndex_);

	// 各変数をリセット
	currentIndex_ = 0;
	instanceCount_ = 0;
}

void BaseDrawer::CreateInstanceResource(unsigned long long resSize, void** data, ComPtr<ID3D12Resource>& res, uint32_t& srvIdx) {
	// インスタンスリソース作成
	res = directX_.GetResourceFactory()->CreateUploadBufferResource(static_cast<size_t>(resSize * numMaxInstance_));

	// SRV確保
	srvIdx = directX_.GetResMgr().srvuav->Allocate();

	// StructuredBuffer用SRV作成
	directX_.GetResMgr().srvuav->CreateSrvStructuredBuffer(srvIdx, res.Get(), numMaxInstance_, static_cast<UINT>(resSize));

	// CPU書き込み用にマップ
	res->Map(0, nullptr, data);
}
