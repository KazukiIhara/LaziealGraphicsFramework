#include "BaseDrawer.h"

#include <cassert>
#include <cstring>

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
	currentIndex_ = 0u;
	instanceCount_ = 0u;
	instanceRenderPassIndices_.clear();
	renderPassRanges_.fill({});
}

uint32_t BaseDrawer::CheckIntstanceCount() {
	// インスタンスカウント計測
	assert(currentIndex_ <= numMaxInstance_);
	instanceCount_ = currentIndex_;

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
	instanceStride_ = static_cast<uint32_t>(param.instResSize);
	instanceRenderPassIndices_.reserve(numMaxInstance_);

	// インスタンスリソース作成
	CreateInstanceResources(param.instResSize, param.instData);

	// 各変数をリセット
	currentIndex_ = 0;
	instanceCount_ = 0;
	const bool initialized = drawRangeResources_.Initialize(
		*directX_.GetResourceFactory(),
		static_cast<size_t>(kRenderPassConstantBufferStride) * kMaxRenderPassCount);
	assert(initialized);
}

void BaseDrawer::AddInstance(const void* data, uint32_t renderPassIndex) {
	assert(data);
	assert(renderPassIndex < kMaxRenderPassCount);
	assert(currentIndex_ < numMaxInstance_);
	std::byte* destination = reinterpret_cast<std::byte*>(*instanceDataTarget_) +
		static_cast<size_t>(currentIndex_) * instanceStride_;
	std::memcpy(destination, data, instanceStride_);
	instanceRenderPassIndices_.push_back(renderPassIndex);
	++currentIndex_;
}

void BaseDrawer::PrepareRenderPassInstances() {
	instanceCount_ = currentIndex_;
	sortedInstanceData_.resize(static_cast<size_t>(instanceStride_) * instanceCount_);
	renderPassRanges_.fill({});
	for (uint32_t renderPassIndex : instanceRenderPassIndices_) {
		++renderPassRanges_.at(renderPassIndex).count;
	}

	uint32_t offset = 0u;
	for (RenderPassInstanceRange& range : renderPassRanges_) {
		range.offset = offset;
		offset += range.count;
	}
	std::array<uint32_t, kMaxRenderPassCount> writeIndices{};
	for (uint32_t i = 0u; i < kMaxRenderPassCount; ++i) {
		writeIndices[i] = renderPassRanges_[i].offset;
	}

	const std::byte* source = reinterpret_cast<const std::byte*>(*instanceDataTarget_);
	for (uint32_t instanceIndex = 0u; instanceIndex < instanceCount_; ++instanceIndex) {
		const uint32_t renderPassIndex = instanceRenderPassIndices_[instanceIndex];
		const uint32_t destinationIndex = writeIndices[renderPassIndex]++;
		std::memcpy(
			sortedInstanceData_.data() + static_cast<size_t>(destinationIndex) * instanceStride_,
			source + static_cast<size_t>(instanceIndex) * instanceStride_,
			instanceStride_);
	}
	if (instanceCount_ != 0u) {
		std::memcpy(
			*instanceDataTarget_,
			sortedInstanceData_.data(),
			static_cast<size_t>(instanceCount_) * instanceStride_);
	}

	std::byte* drawRangeData = drawRangeResources_.GetMappedData(
		directX_.GetCurrentFrameContextIndex());
	for (uint32_t i = 0u; i < kMaxRenderPassCount; ++i) {
		const DrawRangeForGPU data{ .instanceOffset = renderPassRanges_[i].offset };
		std::memcpy(
			drawRangeData + static_cast<size_t>(i) * kRenderPassConstantBufferStride,
			&data,
			sizeof(data));
	}
}

RenderPassInstanceRange BaseDrawer::GetRenderPassInstanceRange(uint32_t renderPassIndex) const {
	return renderPassRanges_.at(renderPassIndex);
}

D3D12_GPU_VIRTUAL_ADDRESS BaseDrawer::GetDrawRangeAddress(uint32_t renderPassIndex) const {
	return drawRangeResources_.GetGPUVirtualAddress(directX_.GetCurrentFrameContextIndex()) +
		static_cast<uint64_t>(renderPassIndex) * kRenderPassConstantBufferStride;
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
