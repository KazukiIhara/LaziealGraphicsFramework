#include "SkyboxSystem.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <utility>

#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/DirectX/ResourceFactory/ResourceFactory.h"
#include "Graphics/DirectX/ViewManagers/SRVUAVManager/SRVUAVManager.h"
#include "Graphics/Pipeline/Manager/PipelineManager.h"
#include "Graphics/Pipeline/Pipeline.h"
#include "LGF/Graphics/Skybox.h"
#include "LGF/Math/MathConstants.h"

using namespace LGF;

namespace {
	constexpr uint32_t kConstantBufferAlignment = 256u;
	constexpr uint32_t kFilterConstantSlotCount = 16u;
	constexpr uint32_t kIrradianceSize = 32u;
	constexpr uint32_t kPrefilteredSize = 128u;
	constexpr uint32_t kPrefilteredMipCount = 8u;
	constexpr uint32_t kBrdfLutSize = 256u;
	constexpr uint32_t kFilterSampleCount = 128u;
	constexpr uint32_t kThreadGroupSize = 8u;
	constexpr char kDiffuseIrradiancePipelineName[] = "DiffuseIrradiance";
	constexpr char kPrefilterEnvironmentPipelineName[] = "PrefilterEnvironment";
	constexpr char kIntegrateBrdfPipelineName[] = "IntegrateBrdf";
	constexpr char kFilterConstantsName[] = "EnvironmentFilterData";

	uint32_t DispatchCount(uint32_t size) {
		return (size + kThreadGroupSize - 1u) / kThreadGroupSize;
	}

	D3D12_RESOURCE_BARRIER MakeTransitionBarrier(
		ID3D12Resource* resource,
		D3D12_RESOURCE_STATES before,
		D3D12_RESOURCE_STATES after) {

		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = resource;
		barrier.Transition.StateBefore = before;
		barrier.Transition.StateAfter = after;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		return barrier;
	}

	D3D12_RESOURCE_BARRIER MakeUavBarrier(ID3D12Resource* resource) {
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		barrier.UAV.pResource = resource;
		return barrier;
	}
}

SkyboxSystem::SkyboxSystem(DirectXCore& directX, PipelineManager& pipelineManager)
	: directX_(directX), pipelineManager_(pipelineManager) {
}

SkyboxSystem::~SkyboxSystem() = default;

bool SkyboxSystem::Initialize() {
	if (!resources_.Initialize(
		*directX_.GetResourceFactory(),
		kConstantBufferAlignment)) {
		return false;
	}
	return filterConstantsResources_.Initialize(
		*directX_.GetResourceFactory(),
		kConstantBufferAlignment * kFilterConstantSlotCount);
}

void SkyboxSystem::BeginFrame() {
	pendingData_ = SkyboxForGPU{};
	pendingSourceTexture_ = nullptr;
}

void SkyboxSystem::EndFrame() {
	SkyboxForGPU* data_ = reinterpret_cast<SkyboxForGPU*>(
		resources_.GetMappedData(directX_.GetCurrentFrameContextIndex()));
	assert(data_);
	if (pendingData_.textureIndex != UINT32_MAX && pendingSourceTexture_) {
		auto found = filteredEnvironments_.find(pendingData_.textureIndex);
		if (found == filteredEnvironments_.end()) {
			if (!CreateFilteredEnvironment(
				pendingData_.textureIndex,
				pendingSourceTexture_)) {
				*data_ = pendingData_;
				return;
			}
			found = filteredEnvironments_.find(pendingData_.textureIndex);
		}

		if (found != filteredEnvironments_.end()) {
			pendingData_.irradianceTextureIndex = found->second.irradianceSrvIndex;
			pendingData_.prefilteredTextureIndex = found->second.prefilteredSrvIndex;
			pendingData_.prefilteredMipCount = found->second.prefilteredMipCount;
			pendingData_.brdfLutTextureIndex = brdfLutSrvIndex_;
		}
	}
	*data_ = pendingData_;
}

void SkyboxSystem::Finalize() {
	filteredEnvironments_.clear();
	brdfLut_.Reset();
	brdfLutSrvIndex_ = UINT32_MAX;
	filterConstantsResources_.Finalize();
	resources_.Finalize();
}

void SkyboxSystem::SetSkybox(const Skybox& skybox) {
	assert(skybox.texture_.IsValid());
	assert(!skybox.texture_.isCubeMap_);
	assert(skybox.texture_.resource_);

	pendingData_.textureIndex = skybox.texture_.srvIndex_;
	pendingData_.intensity = std::isfinite(skybox.intensity)
		? std::max(skybox.intensity, 0.0f)
		: 1.0f;
	pendingData_.rotation = std::isfinite(skybox.rotation)
		? std::remainder(skybox.rotation, Math::TWO_PI)
		: 0.0f;
	pendingSourceTexture_ = skybox.texture_.resource_;
}

void SkyboxSystem::Bind(ID3D12GraphicsCommandList* commandList, uint32_t rootParamIdx) const {
	assert(commandList);
	commandList->SetGraphicsRootConstantBufferView(
		static_cast<UINT>(rootParamIdx),
		resources_.GetGPUVirtualAddress(directX_.GetCurrentFrameContextIndex()));
}

bool SkyboxSystem::CreateBrdfLut() {
	if (brdfLut_) {
		return true;
	}

	auto* views = directX_.GetResMgr().srvuav;
	if (!views->IsLowerViewMax()) {
		return false;
	}
	brdfLut_ = directX_.GetResourceFactory()->CreateUAVTextureResource(
		kBrdfLutSize, kBrdfLutSize, 1u, 1u, DXGI_FORMAT_R16G16_FLOAT);
	if (!brdfLut_) {
		return false;
	}

	brdfLutSrvIndex_ = views->Allocate();
	views->CreateSrvTexture2d(brdfLutSrvIndex_, brdfLut_.Get(), DXGI_FORMAT_R16G16_FLOAT, 1u);
	if (!views->IsLowerViewMax()) {
		return false;
	}
	const uint32_t uavIndex = views->Allocate();
	views->CreateUavTexture2d(uavIndex, brdfLut_.Get(), DXGI_FORMAT_R16G16_FLOAT, 0u);

	Pipeline* pipeline = pipelineManager_.GetPipeline(kIntegrateBrdfPipelineName);
	assert(pipeline && pipeline->IsValid());
	DispatchFilter(
		*pipeline,
		EnvironmentFilterForGPU{
			.outputTextureIndex = uavIndex,
			.outputSize = kBrdfLutSize,
			.sampleCount = kFilterSampleCount,
		},
		kPrefilteredMipCount + 1u,
		kBrdfLutSize,
		kBrdfLutSize,
		1u);

	ID3D12GraphicsCommandList* commandList = directX_.GetCommandList();
	const D3D12_RESOURCE_BARRIER uavBarrier = MakeUavBarrier(brdfLut_.Get());
	commandList->ResourceBarrier(1u, &uavBarrier);
	const D3D12_RESOURCE_BARRIER transition = MakeTransitionBarrier(
		brdfLut_.Get(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	commandList->ResourceBarrier(1u, &transition);
	return true;
}

bool SkyboxSystem::CreateFilteredEnvironment(
	uint32_t sourceTextureIndex,
	ID3D12Resource* sourceTexture) {

	assert(sourceTexture);
	if (!CreateBrdfLut()) {
		return false;
	}

	auto* views = directX_.GetResMgr().srvuav;
	FilteredEnvironment environment{};
	environment.prefilteredMipCount = kPrefilteredMipCount;
	environment.irradiance = directX_.GetResourceFactory()->CreateUAVTextureResource(
		kIrradianceSize, kIrradianceSize, 6u, 1u, DXGI_FORMAT_R16G16B16A16_FLOAT);
	environment.prefiltered = directX_.GetResourceFactory()->CreateUAVTextureResource(
		kPrefilteredSize,
		kPrefilteredSize,
		6u,
		static_cast<uint16_t>(kPrefilteredMipCount),
		DXGI_FORMAT_R16G16B16A16_FLOAT);
	if (!environment.irradiance || !environment.prefiltered) {
		return false;
	}

	if (!views->IsLowerViewMax()) {
		return false;
	}
	environment.irradianceSrvIndex = views->Allocate();
	views->CreateSrvTextureCubeMap(
		environment.irradianceSrvIndex,
		environment.irradiance.Get(),
		DXGI_FORMAT_R16G16B16A16_FLOAT);
	if (!views->IsLowerViewMax()) {
		return false;
	}
	environment.prefilteredSrvIndex = views->Allocate();
	views->CreateSrvTextureCubeMap(
		environment.prefilteredSrvIndex,
		environment.prefiltered.Get(),
		DXGI_FORMAT_R16G16B16A16_FLOAT);

	if (!views->IsLowerViewMax()) {
		return false;
	}
	const uint32_t irradianceUavIndex = views->Allocate();
	views->CreateUavTexture2dArray(
		irradianceUavIndex,
		environment.irradiance.Get(),
		DXGI_FORMAT_R16G16B16A16_FLOAT,
		0u,
		6u);

	uint32_t prefilteredUavIndices[kPrefilteredMipCount]{};
	for (uint32_t mip = 0u; mip < kPrefilteredMipCount; ++mip) {
		if (!views->IsLowerViewMax()) {
			return false;
		}
		prefilteredUavIndices[mip] = views->Allocate();
		views->CreateUavTexture2dArray(
			prefilteredUavIndices[mip],
			environment.prefiltered.Get(),
			DXGI_FORMAT_R16G16B16A16_FLOAT,
			mip,
			6u);
	}

	ID3D12GraphicsCommandList* commandList = directX_.GetCommandList();
	const D3D12_RESOURCE_BARRIER sourceToCompute = MakeTransitionBarrier(
		sourceTexture,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	commandList->ResourceBarrier(1u, &sourceToCompute);

	Pipeline* irradiancePipeline = pipelineManager_.GetPipeline(kDiffuseIrradiancePipelineName);
	assert(irradiancePipeline && irradiancePipeline->IsValid());
	DispatchFilter(
		*irradiancePipeline,
		EnvironmentFilterForGPU{
			.sourceTextureIndex = sourceTextureIndex,
			.outputTextureIndex = irradianceUavIndex,
			.outputSize = kIrradianceSize,
			.sampleCount = kFilterSampleCount,
		},
		0u,
		kIrradianceSize,
		kIrradianceSize,
		6u);

	Pipeline* prefilterPipeline = pipelineManager_.GetPipeline(kPrefilterEnvironmentPipelineName);
	assert(prefilterPipeline && prefilterPipeline->IsValid());
	for (uint32_t mip = 0u; mip < kPrefilteredMipCount; ++mip) {
		const uint32_t mipSize = std::max(kPrefilteredSize >> mip, 1u);
		const float roughness = static_cast<float>(mip) /
			static_cast<float>(kPrefilteredMipCount - 1u);
		DispatchFilter(
			*prefilterPipeline,
			EnvironmentFilterForGPU{
				.sourceTextureIndex = sourceTextureIndex,
				.outputTextureIndex = prefilteredUavIndices[mip],
				.outputSize = mipSize,
				.sampleCount = kFilterSampleCount,
				.roughness = roughness,
			},
			mip + 1u,
			mipSize,
			mipSize,
			6u);
	}

	const D3D12_RESOURCE_BARRIER uavBarriers[] = {
		MakeUavBarrier(environment.irradiance.Get()),
		MakeUavBarrier(environment.prefiltered.Get()),
	};
	commandList->ResourceBarrier(_countof(uavBarriers), uavBarriers);
	const D3D12_RESOURCE_BARRIER transitions[] = {
		MakeTransitionBarrier(
			environment.irradiance.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
		MakeTransitionBarrier(
			environment.prefiltered.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
		MakeTransitionBarrier(
			sourceTexture,
			D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
	};
	commandList->ResourceBarrier(_countof(transitions), transitions);

	filteredEnvironments_.emplace(sourceTextureIndex, std::move(environment));
	return true;
}

void SkyboxSystem::DispatchFilter(
	Pipeline& pipeline,
	const EnvironmentFilterForGPU& constants,
	uint32_t constantSlot,
	uint32_t width,
	uint32_t height,
	uint32_t depth) {

	assert(constantSlot < kFilterConstantSlotCount);
	const uint32_t frameIndex = directX_.GetCurrentFrameContextIndex();
	std::byte* filterConstantsData = filterConstantsResources_.GetMappedData(frameIndex);
	assert(filterConstantsData);
	std::memcpy(
		filterConstantsData + static_cast<size_t>(constantSlot) * kConstantBufferAlignment,
		&constants,
		sizeof(constants));

	ID3D12GraphicsCommandList* commandList = directX_.GetCommandList();
	commandList->SetPipelineState(pipeline.GetPipelineState());
	commandList->SetComputeRootSignature(pipeline.GetRootSignature());
	commandList->SetComputeRootConstantBufferView(
		pipeline.GetRootIndex(kFilterConstantsName),
		filterConstantsResources_.GetGPUVirtualAddress(frameIndex) +
			static_cast<UINT64>(constantSlot) * kConstantBufferAlignment);
	commandList->Dispatch(DispatchCount(width), DispatchCount(height), depth);
}
