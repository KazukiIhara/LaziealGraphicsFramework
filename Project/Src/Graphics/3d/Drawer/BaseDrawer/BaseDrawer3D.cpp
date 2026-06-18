#include "BaseDrawer3D.h"

#include "DirectX/Core/DirectXCore.h"
#include "Graphics/DirectX/ResourceFactory/ResourceFactory.h"
#include "Graphics/DirectX/ViewManagers/SRVUAVManager/SRVUAVManager.h"

using namespace LGF;

BaseDrawer3D::BaseDrawer3D(Graphics3DContext context) :
	context_(context) {
}

BaseDrawer3D::~BaseDrawer3D() = default;

Graphics3DContext BaseDrawer3D::GetContext() const {
	return context_;
}

Pipeline* BaseDrawer3D::GetPipeline() {
	return pipeline_;
}

void BaseDrawer3D::SetPipeline(Pipeline* pipeline) {
	pipeline_ = pipeline;
}

void BaseDrawer3D::Initialize(InitParam param) {
	// インスタンスの最大数を取得
	numMaxInstance_ = param.numMaxInstance;

	// インスタンスリソース作成
	CreateInstanceResource(param.instResSize, param.instData, instanceResource_, instanceSrvIndex_);

	// 各変数をリセット
	currentIndex_ = 0;
	instanceCount_ = 0;
}

void BaseDrawer3D::CreateInstanceResource(unsigned long long resSize, void** data, ComPtr<ID3D12Resource>& res, uint32_t& srvIdx) {
	// インスタンスリソース作成
	res = GetContext().directX.GetResourceFactory()->CreateUploadBufferResource(static_cast<size_t>(resSize * numMaxInstance_));

	// SRV確保
	srvIdx = GetContext().directX.GetResMgr().srvuav->Allocate();

	// StructuredBuffer用SRV作成
	GetContext().directX.GetResMgr().srvuav->CreateSrvStructuredBuffer(srvIdx, res.Get(), numMaxInstance_, static_cast<UINT>(resSize));

	// CPU書き込み用にマップ
	res->Map(0, nullptr, data);
}