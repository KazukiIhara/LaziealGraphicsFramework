#include "BaseDrawer3D.h"

#include "DirectX/Core/DirectXCore.h"
#include "Graphics/DirectX/ResourceFactory/ResourceFactory.h"
#include "Graphics/DirectX/ViewManagers/SRVUAVManager/SRVUAVManager.h"

using namespace LGF;

BaseDrawer3D::BaseDrawer3D(DirectXCore* directx) :
	directX_(directx) {

}

BaseDrawer3D::~BaseDrawer3D() = default;

void BaseDrawer3D::Initialize(InitParam param) {
	// インスタンスの最大数を取得
	numMaxInstance_ = param.numMaxInstance;

	// インスタンスリソース作成
	CreateInstanceResource(param.instResSize, param.instData, instanceResource_, instanceSrvIndex_);

	// 各変数をリセット
	currentIndex_ = 0;
	instanceCount_ = 0;

}

void BaseDrawer3D::SetPipeline(Pipeline* pipeline) {
	pipeline_ = pipeline;
}

DirectXCore* BaseDrawer3D::GetDirectX() {
	return directX_;
}

void BaseDrawer3D::CreateInstanceResource(unsigned long long resSize, void** data, ComPtr<ID3D12Resource>& res, uint32_t& srvIdx) {
	// インスタンスリソース作成
	res = GetDirectX()->GetResourceFactory()->CreateUploadBufferResource(static_cast<size_t>(resSize * numMaxInstance_));

	// SRV確保
	srvIdx = GetDirectX()->GetResMgr().srvuav->Allocate();

	// StructuredBuffer用SRV作成
	GetDirectX()->GetResMgr().srvuav->CreateSrvStructuredBuffer(srvIdx, res.Get(), numMaxInstance_, static_cast<UINT>(resSize));

	// CPU書き込み用にマップ
	res->Map(0, nullptr, data);
}
