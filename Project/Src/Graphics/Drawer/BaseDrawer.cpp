#include "BaseDrawer.h"

#include <cassert>

#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/DirectX/ResourceFactory/ResourceFactory.h"
#include "Graphics/DirectX/ViewManagers/SRVUAVManager/SRVUAVManager.h"

using namespace LGF;

BaseDrawer::BaseDrawer(DirectXCore& directX, PipelineManager& pipelineMgr) :
	directX_(directX), pipelineMgr_(pipelineMgr) {
}

BaseDrawer::~BaseDrawer() = default;

void BaseDrawer::BeginFrame() {
	const uint32_t frameIndex = directX_.GetCurrentFrameContextIndex();
	assert(instanceDataTarget_);
	*instanceDataTarget_ = instanceResources_.GetMappedData(frameIndex);
	instanceSrvIndex_ = instanceSrvIndices_.at(frameIndex);
}

uint32_t BaseDrawer::CheckIntstanceCount() {
	// インスタンスカウント計測
	assert(currentIndex_ <= numMaxInstance_);
	instanceCount_ = currentIndex_;
	currentIndex_ = 0;

	// インスタンスの数を返す
	return instanceCount_;
}

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
	CreateInstanceResources(param.instResSize, param.instData);

	// 各変数をリセット
	currentIndex_ = 0;
	instanceCount_ = 0;
}

void BaseDrawer::CreateInstanceResources(unsigned long long resSize, void** data) {
	// インスタンスリソース作成
	instanceDataTarget_ = data;
	const bool initialized = instanceResources_.Initialize(
		*directX_.GetResourceFactory(),
		static_cast<size_t>(resSize * numMaxInstance_));
	assert(initialized);

	// SRV確保
	for (uint32_t frameIndex = 0u; frameIndex < kFrameContextCount; ++frameIndex) {
		const uint32_t srvIndex = directX_.GetResMgr().srvuav->Allocate();
		instanceSrvIndices_[frameIndex] = srvIndex;

		// StructuredBuffer用SRV作成
		directX_.GetResMgr().srvuav->CreateSrvStructuredBuffer(
			srvIndex,
			instanceResources_.GetResource(frameIndex),
			numMaxInstance_,
			static_cast<UINT>(resSize));
	}

	// CPU書き込み用にマップ
	BeginFrame();
}
