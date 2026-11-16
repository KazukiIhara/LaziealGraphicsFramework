#include "PostProcessSystem.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "Core/Logger/Logger.h"
#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/DirectX/DepthStencil/DepthStencil.h"
#include "Graphics/Pipeline/Manager/PipelineManager.h"
#include "Graphics/Pipeline/Pipeline.h"
#include "Graphics/PostProcess/JsonLoader/PostProcessJsonLoader.h"
#include "Graphics/RenderTexture/RenderTexture.h"

using namespace LGF;

namespace {

	constexpr size_t kConstantBufferAlignment = 256u;
	constexpr char kToneMappingExecutor[] = "ToneMapping";
	constexpr char kFullscreenExecutor[] = "Fullscreen";
	constexpr char kBloomExecutor[] = "Bloom";
	constexpr char kPresentPipeline[] = "PostProcessPresent";
	constexpr char kInputColorShaderName[] = "gInputColor";
	constexpr char kSceneColorShaderName[] = "gSceneColorHDR";
	constexpr char kSceneDepthShaderName[] = "gSceneDepth";
	constexpr char kSceneNormalShaderName[] = "gSceneNormal";
	constexpr char kSceneAlbedoShaderName[] = "gSceneAlbedo";
	constexpr char kSceneMaterialShaderName[] = "gSceneMaterial";
	constexpr char kBloomTextureShaderName[] = "gBloomTexture";
	constexpr char kParameterShaderName[] = "PostProcessParameters";
	constexpr char kContextShaderName[] = "PostProcessContext";

	float* GetComponents(Vector4& value) {
		return &value.x;
	}

	const float* GetComponents(const Vector4& value) {
		return &value.x;
	}

	void WriteParameter(PostProcessParametersForGPU& destination, const PostProcessParameterDefinition& definition, const float* values) {
		float* destinationComponents = GetComponents(destination.parameters[definition.slot]);
		for (uint32_t index = 0u; index < definition.componentCount; ++index) {
			destinationComponents[definition.componentOffset + index] = values[index];
		}
	}

	void CopyParameter(PostProcessParametersForGPU& destination, const PostProcessParametersForGPU& source, const PostProcessParameterDefinition& definition) {
		float* destinationComponents = GetComponents(destination.parameters[definition.slot]);
		const float* sourceComponents = GetComponents(source.parameters[definition.slot]);
		for (uint32_t index = 0u; index < definition.componentCount; ++index) {
			destinationComponents[definition.componentOffset + index] =
				sourceComponents[definition.componentOffset + index];
		}
	}

	std::pair<const float*, uint32_t> GetParameterValue(const PostProcessParameterValue& value, Vector4& storage) {
		return std::visit([&storage](const auto& current) -> std::pair<const float*, uint32_t> {
			using T = std::decay_t<decltype(current)>;
			if constexpr (std::is_same_v<T, float>) {
				storage = { current, 0.0f, 0.0f, 0.0f };
				return { &storage.x, 1u };
			} else if constexpr (std::is_same_v<T, Vector2>) {
				storage = { current.x, current.y, 0.0f, 0.0f };
				return { &storage.x, 2u };
			} else if constexpr (std::is_same_v<T, Vector3>) {
				storage = { current.x, current.y, current.z, 0.0f };
				return { &storage.x, 3u };
			} else {
				storage = current;
				return { &storage.x, 4u };
			}
		}, value);
	}

}

PostProcessSystem::PostProcessSystem(DirectXCore& directX, PipelineManager& pipelineManager, std::filesystem::path assetRoot)
	: directX_(directX), pipelineManager_(pipelineManager), assetRoot_(std::move(assetRoot)) {
}

PostProcessSystem::~PostProcessSystem() {
	Finalize();
}

bool PostProcessSystem::Initialize() {
	try {
		PostProcessJsonLoader loader;
		definitions_ = loader.LoadEffectDefinitions(assetRoot_ / "JsonData" / "PostEffect");
	} catch (const std::exception& error) {
		Logger::Log(error.what());
		return false;
	}

	if (!contextResources_.Initialize(*directX_.GetResourceFactory(), kConstantBufferAlignment) ||
		!fallbackParameterResources_.Initialize(*directX_.GetResourceFactory(), kConstantBufferAlignment) ||
		!CreateRenderTextures(directX_.GetRenderWidth(), directX_.GetRenderHeight())) {
		Finalize();
		return false;
	}

	initialized_ = true;
	SetExposure(0.0f);
	return true;
}

void PostProcessSystem::Finalize() {
	profiles_.clear();
	freeProfileIndices_.clear();
	definitions_.clear();
	activeProfile_ = {};
	hdrPing_.reset();
	hdrPong_.reset();
	ldrPing_.reset();
	ldrPong_.reset();
	bloomPing_.reset();
	bloomPong_.reset();
	contextResources_.Finalize();
	fallbackParameterResources_.Finalize();
	initialized_ = false;
}

bool PostProcessSystem::Resize(uint32_t width, uint32_t height) {
	if (!initialized_ || width == 0u || height == 0u) {
		return false;
	}
	return CreateRenderTextures(width, height);
}

bool PostProcessSystem::CreateRenderTextures(uint32_t width, uint32_t height) {
	const uint32_t bloomWidth = std::max(width / 2u, 1u);
	const uint32_t bloomHeight = std::max(height / 2u, 1u);
	if (hdrPing_ && hdrPong_ && ldrPing_ && ldrPong_ && bloomPing_ && bloomPong_) {
		return hdrPing_->Resize(width, height) &&
			hdrPong_->Resize(width, height) &&
			ldrPing_->Resize(width, height) &&
			ldrPong_->Resize(width, height) &&
			bloomPing_->Resize(bloomWidth, bloomHeight) &&
			bloomPong_->Resize(bloomWidth, bloomHeight);
	}

	hdrPing_ = CreateRenderTexture(width, height, true);
	hdrPong_ = CreateRenderTexture(width, height, true);
	ldrPing_ = CreateRenderTexture(width, height, false);
	ldrPong_ = CreateRenderTexture(width, height, false);
	bloomPing_ = CreateRenderTexture(bloomWidth, bloomHeight, true);
	bloomPong_ = CreateRenderTexture(bloomWidth, bloomHeight, true);
	return hdrPing_ && hdrPong_ && ldrPing_ && ldrPong_ && bloomPing_ && bloomPong_;
}

std::unique_ptr<RenderTexture> PostProcessSystem::CreateRenderTexture(uint32_t width, uint32_t height, bool hdr) {
	DirectXCore::ResMgr managers = directX_.GetResMgr();
	auto texture = std::make_unique<RenderTexture>(
		*directX_.GetResourceFactory(),
		*managers.rtv,
		*managers.srvuav);

	RenderTextureDesc desc{};
	desc.width = width;
	desc.height = height;
	desc.clearColor = {};
	if (hdr) {
		desc.resourceFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
		desc.rtvFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
		desc.srvFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
	} else {
		desc.resourceFormat = DXGI_FORMAT_R8G8B8A8_TYPELESS;
		desc.rtvFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.srvFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	}

	if (!texture->Initialize(desc)) {
		return nullptr;
	}
	return texture;
}

PostProcessProfileHandle PostProcessSystem::CreateProfile(const std::filesystem::path& filePath) {
	std::filesystem::path resolvedPath = filePath;
	if (resolvedPath.is_relative()) {
		resolvedPath = resolvedPath.has_parent_path()
			? assetRoot_ / resolvedPath
			: assetRoot_ / "JsonData" / "PostProcess" / resolvedPath;
	}

	PostProcessProfileData data{};
	try {
		PostProcessJsonLoader loader;
		data = loader.LoadProfile(resolvedPath.lexically_normal(), definitions_);
	} catch (const std::exception& error) {
		Logger::Log(error.what());
		return {};
	}

	auto profile = std::make_unique<ProfileInstance>();
	profile->name = std::move(data.name);
	profile->effects.reserve(data.effects.size());
	for (const PostProcessProfileEffectData& effectData : data.effects) {
		const auto definitionIterator = definitions_.find(effectData.effect);
		assert(definitionIterator != definitions_.end());
		const PostEffectDefinition& definition = definitionIterator->second;

		EffectInstance effect{};
		effect.id = effectData.id;
		effect.definition = &definition;
		effect.baseEnabled = effectData.enabled;
		effect.enabled = effectData.enabled;
		for (const auto& [name, parameter] : definition.parameters) {
			(void)name;
			WriteParameter(effect.baseParameters, parameter, GetComponents(parameter.defaultValue));
		}
		for (const auto& [name, value] : effectData.parameters) {
			const PostProcessParameterDefinition& parameter = definition.parameters.at(name);
			Vector4 storage{};
			const auto [values, valueCount] = GetParameterValue(value, storage);
			if (valueCount != parameter.componentCount) {
				Logger::Log("Post Process Profile parameter type mismatch: " + effect.id + "." + name);
				return {};
			}
			WriteParameter(effect.baseParameters, parameter, values);
		}
		effect.parameters = effect.baseParameters;
		effect.parameterResources = std::make_unique<FrameBufferedUploadBuffer>();
		if (!effect.parameterResources->Initialize(*directX_.GetResourceFactory(), kConstantBufferAlignment)) {
			return {};
		}
		profile->effects.push_back(std::move(effect));
	}

	uint32_t index = 0u;
	if (!freeProfileIndices_.empty()) {
		index = freeProfileIndices_.back();
		freeProfileIndices_.pop_back();
		profiles_[index].instance = std::move(profile);
	} else {
		index = static_cast<uint32_t>(profiles_.size());
		profiles_.push_back({ .instance = std::move(profile) });
	}
	return PostProcessProfileHandle{ index, profiles_[index].generation };
}

bool PostProcessSystem::RemoveProfile(PostProcessProfileHandle handle) {
	ProfileInstance* profile = FindProfile(handle);
	if (!profile) {
		return false;
	}
	(void)profile;
	if (activeProfile_ == handle) {
		activeProfile_ = {};
	}
	ProfileSlot& slot = profiles_[handle.index];
	slot.instance.reset();
	++slot.generation;
	if (slot.generation == 0u) {
		slot.generation = 1u;
	}
	freeProfileIndices_.push_back(handle.index);
	return true;
}

bool PostProcessSystem::IsValid(PostProcessProfileHandle handle) const {
	return FindProfile(handle) != nullptr;
}

bool PostProcessSystem::SetActiveProfile(PostProcessProfileHandle handle) {
	if (!IsValid(handle)) {
		return false;
	}
	activeProfile_ = handle;
	return true;
}

void PostProcessSystem::ResetActiveProfile() {
	activeProfile_ = {};
}

PostProcessSystem::ProfileInstance* PostProcessSystem::FindProfile(PostProcessProfileHandle handle) {
	return const_cast<ProfileInstance*>(std::as_const(*this).FindProfile(handle));
}

const PostProcessSystem::ProfileInstance* PostProcessSystem::FindProfile(PostProcessProfileHandle handle) const {
	if (!handle.IsValid() || handle.index >= profiles_.size()) {
		return nullptr;
	}
	const ProfileSlot& slot = profiles_[handle.index];
	return slot.generation == handle.generation ? slot.instance.get() : nullptr;
}

PostProcessSystem::EffectInstance* PostProcessSystem::FindEffect(PostProcessProfileHandle handle, std::string_view effectId) {
	ProfileInstance* profile = FindProfile(handle);
	if (!profile) {
		return nullptr;
	}
	for (EffectInstance& effect : profile->effects) {
		if (effect.id == effectId) {
			return &effect;
		}
	}
	return nullptr;
}

bool PostProcessSystem::SetEffectEnabled(PostProcessProfileHandle handle, std::string_view effectId, bool enabled) {
	EffectInstance* effect = FindEffect(handle, effectId);
	if (!effect || (!enabled && effect->definition->stage == PostProcessStage::ToneMapping)) {
		return false;
	}
	effect->enabled = enabled;
	return true;
}

bool PostProcessSystem::SetParameter(PostProcessProfileHandle handle, std::string_view effectId, std::string_view parameterName, float value) {
	return SetParameter(handle, effectId, parameterName, &value, 1u);
}

bool PostProcessSystem::SetParameter(PostProcessProfileHandle handle, std::string_view effectId, std::string_view parameterName, const Vector2& value) {
	return SetParameter(handle, effectId, parameterName, &value.x, 2u);
}

bool PostProcessSystem::SetParameter(PostProcessProfileHandle handle, std::string_view effectId, std::string_view parameterName, const Vector3& value) {
	return SetParameter(handle, effectId, parameterName, &value.x, 3u);
}

bool PostProcessSystem::SetParameter(PostProcessProfileHandle handle, std::string_view effectId, std::string_view parameterName, const Vector4& value) {
	return SetParameter(handle, effectId, parameterName, &value.x, 4u);
}

bool PostProcessSystem::SetParameter(PostProcessProfileHandle handle, std::string_view effectId, std::string_view parameterName, const float* values, uint32_t valueCount) {
	EffectInstance* effect = FindEffect(handle, effectId);
	if (!effect || !values) {
		return false;
	}
	const auto parameterIterator = effect->definition->parameters.find(std::string(parameterName));
	if (parameterIterator == effect->definition->parameters.end()) {
		return false;
	}
	const PostProcessParameterDefinition& definition = parameterIterator->second;
	if (definition.componentCount != valueCount) {
		return false;
	}
	for (uint32_t index = 0u; index < valueCount; ++index) {
		if (!std::isfinite(values[index]) ||
			(definition.minimum && values[index] < *definition.minimum) ||
			(definition.maximum && values[index] > *definition.maximum)) {
			return false;
		}
	}
	WriteParameter(effect->parameters, definition, values);
	effect->overriddenParameters.emplace(parameterName);
	return true;
}

bool PostProcessSystem::ClearParameterOverride(PostProcessProfileHandle handle, std::string_view effectId, std::string_view parameterName) {
	EffectInstance* effect = FindEffect(handle, effectId);
	if (!effect) {
		return false;
	}
	const auto parameterIterator = effect->definition->parameters.find(std::string(parameterName));
	if (parameterIterator == effect->definition->parameters.end()) {
		return false;
	}
	CopyParameter(effect->parameters, effect->baseParameters, parameterIterator->second);
	effect->overriddenParameters.erase(std::string(parameterName));
	return true;
}

bool PostProcessSystem::ClearOverrides(PostProcessProfileHandle handle) {
	ProfileInstance* profile = FindProfile(handle);
	if (!profile) {
		return false;
	}
	for (EffectInstance& effect : profile->effects) {
		effect.parameters = effect.baseParameters;
		effect.enabled = effect.baseEnabled;
		effect.overriddenParameters.clear();
	}
	return true;
}

void PostProcessSystem::SetExposure(float exposureEV) {
	exposureEV_ = std::isfinite(exposureEV) ? exposureEV : 0.0f;
	fallbackParameters_.parameters[0].x = exposureEV_;
	ProfileInstance* profile = FindProfile(activeProfile_);
	if (!profile) {
		return;
	}
	for (EffectInstance& effect : profile->effects) {
		if (effect.definition->stage != PostProcessStage::ToneMapping) {
			continue;
		}
		const auto parameter = effect.definition->parameters.find("ExposureEV");
		if (parameter != effect.definition->parameters.end()) {
			WriteParameter(effect.parameters, parameter->second, &exposureEV_);
			effect.overriddenParameters.emplace("ExposureEV");
		}
		return;
	}
}

bool PostProcessSystem::Execute() {
	if (!initialized_) {
		return false;
	}
	ProfileInstance* profile = FindProfile(activeProfile_);
	return profile ? ExecuteProfile(*profile) : ExecuteFallback();
}

bool PostProcessSystem::ExecuteProfile(ProfileInstance& profile) {
	RenderTexture* hdrCurrent = directX_.GetSceneColor();
	RenderTexture* ldrCurrent = nullptr;
	if (!hdrCurrent) {
		return false;
	}

	const uint32_t frameIndex = directX_.GetCurrentFrameContextIndex();
	for (EffectInstance& effect : profile.effects) {
		if (!effect.enabled) {
			continue;
		}
		std::memcpy(
			effect.parameterResources->GetMappedData(frameIndex),
			&effect.parameters,
			sizeof(effect.parameters));

		if (effect.definition->executor == kBloomExecutor) {
			RenderTexture* output = hdrCurrent == hdrPing_.get() ? hdrPong_.get() : hdrPing_.get();
			if (!ExecuteBloom(effect, *hdrCurrent, *output)) {
				return false;
			}
			hdrCurrent = output;
		} else if (effect.definition->executor == kToneMappingExecutor) {
			if (!ExecuteToneMapping(effect, *hdrCurrent, *ldrPing_)) {
				return false;
			}
			ldrCurrent = ldrPing_.get();
		} else if (effect.definition->executor == kFullscreenExecutor) {
			if (effect.definition->stage == PostProcessStage::BeforeToneMapping) {
				RenderTexture* output = hdrCurrent == hdrPing_.get() ? hdrPong_.get() : hdrPing_.get();
				if (!ExecuteFullscreenEffect(effect, *hdrCurrent, *output)) {
					return false;
				}
				hdrCurrent = output;
			} else if (effect.definition->stage == PostProcessStage::AfterToneMapping && ldrCurrent) {
				RenderTexture* output = ldrCurrent == ldrPing_.get() ? ldrPong_.get() : ldrPing_.get();
				if (!ExecuteFullscreenEffect(effect, *ldrCurrent, *output)) {
					return false;
				}
				ldrCurrent = output;
			} else {
				return false;
			}
		} else {
			Logger::Log("Unknown Post Process Executor: " + effect.definition->executor);
			return false;
		}
	}

	return ldrCurrent && Present(*ldrCurrent);
}

bool PostProcessSystem::ExecuteFallback() {
	RenderTexture* sceneColor = directX_.GetSceneColor();
	Pipeline* toneMapping = FindPipeline("PostProcessToneMapping");
	if (!sceneColor || !toneMapping) {
		return false;
	}
	const uint32_t frameIndex = directX_.GetCurrentFrameContextIndex();
	std::memcpy(
		fallbackParameterResources_.GetMappedData(frameIndex),
		&fallbackParameters_,
		sizeof(fallbackParameters_));
	if (!DrawFullscreen(
		*toneMapping,
		*sceneColor,
		*ldrPing_,
		fallbackParameterResources_.GetGPUVirtualAddress(frameIndex))) {
		return false;
	}
	return Present(*ldrPing_);
}

bool PostProcessSystem::ExecuteFullscreenEffect(EffectInstance& effect, RenderTexture& input, RenderTexture& output) {
	Pipeline* pipeline = FindPipeline(effect.definition->pipeline);
	if (!pipeline) {
		return false;
	}
	return DrawFullscreen(
		*pipeline,
		input,
		output,
		effect.parameterResources->GetGPUVirtualAddress(directX_.GetCurrentFrameContextIndex()));
}

bool PostProcessSystem::ExecuteToneMapping(EffectInstance& effect, RenderTexture& input, RenderTexture& output) {
	return ExecuteFullscreenEffect(effect, input, output);
}

bool PostProcessSystem::ExecuteBloom(EffectInstance& effect, RenderTexture& input, RenderTexture& output) {
	const auto getPipeline = [this, &effect](std::string_view name) -> Pipeline* {
		const auto iterator = effect.definition->pipelines.find(std::string(name));
		return iterator == effect.definition->pipelines.end() ? nullptr : FindPipeline(iterator->second);
	};
	Pipeline* extract = getPipeline("Extract");
	Pipeline* blurHorizontal = getPipeline("BlurHorizontal");
	Pipeline* blurVertical = getPipeline("BlurVertical");
	Pipeline* composite = getPipeline("Composite");
	if (!extract || !blurHorizontal || !blurVertical || !composite) {
		return false;
	}

	const D3D12_GPU_VIRTUAL_ADDRESS parameters =
		effect.parameterResources->GetGPUVirtualAddress(directX_.GetCurrentFrameContextIndex());
	if (!DrawFullscreen(*extract, input, *bloomPing_, parameters) ||
		!DrawFullscreen(*blurHorizontal, *bloomPing_, *bloomPong_, parameters) ||
		!DrawFullscreen(*blurVertical, *bloomPong_, *bloomPing_, parameters)) {
		return false;
	}

	ID3D12GraphicsCommandList* commandList = directX_.GetCommandList();
	input.Transition(commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	bloomPing_->Transition(commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	output.Transition(commandList, D3D12_RESOURCE_STATE_RENDER_TARGET);
	directX_.SetViewportAndScissor(output.GetWidth(), output.GetHeight());
	const D3D12_CPU_DESCRIPTOR_HANDLE rtv = output.GetRTVHandleCPU();
	commandList->OMSetRenderTargets(1u, &rtv, false, nullptr);
	commandList->SetPipelineState(composite->GetPipelineState());
	commandList->SetGraphicsRootSignature(composite->GetRootSignature());
	commandList->SetGraphicsRootDescriptorTable(
		composite->GetRootIndex(kInputColorShaderName),
		input.GetSRVHandleGPU());
	commandList->SetGraphicsRootDescriptorTable(
		composite->GetRootIndex(kBloomTextureShaderName),
		bloomPing_->GetSRVHandleGPU());
	if (const auto rootIndex = composite->FindRootIndex(kParameterShaderName)) {
		commandList->SetGraphicsRootConstantBufferView(*rootIndex, parameters);
	}
	directX_.GetCommandList6()->DispatchMesh(1u, 1u, 1u);
	output.Transition(commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	return true;
}

bool PostProcessSystem::DrawFullscreen(Pipeline& pipeline, RenderTexture& input, RenderTexture& output, D3D12_GPU_VIRTUAL_ADDRESS parameterAddress) {
	ID3D12GraphicsCommandList* commandList = directX_.GetCommandList();
	input.Transition(commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	output.Transition(commandList, D3D12_RESOURCE_STATE_RENDER_TARGET);
	directX_.SetViewportAndScissor(output.GetWidth(), output.GetHeight());
	const D3D12_CPU_DESCRIPTOR_HANDLE rtv = output.GetRTVHandleCPU();
	commandList->OMSetRenderTargets(1u, &rtv, false, nullptr);
	commandList->SetPipelineState(pipeline.GetPipelineState());
	commandList->SetGraphicsRootSignature(pipeline.GetRootSignature());
	BindSceneInputs(pipeline, input);
	if (const auto rootIndex = pipeline.FindRootIndex(kParameterShaderName)) {
		commandList->SetGraphicsRootConstantBufferView(*rootIndex, parameterAddress);
	}
	UpdateContext(input, output);
	if (const auto rootIndex = pipeline.FindRootIndex(kContextShaderName)) {
		commandList->SetGraphicsRootConstantBufferView(
			*rootIndex,
			contextResources_.GetGPUVirtualAddress(directX_.GetCurrentFrameContextIndex()));
	}
	directX_.GetCommandList6()->DispatchMesh(1u, 1u, 1u);
	output.Transition(commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	return true;
}

void PostProcessSystem::BindSceneInputs(Pipeline& pipeline, RenderTexture& input) {
	ID3D12GraphicsCommandList* commandList = directX_.GetCommandList();
	const auto bindTexture = [commandList, &pipeline](const char* name, RenderTexture* texture) {
		const std::optional<uint32_t> rootIndex = pipeline.FindRootIndex(name);
		if (!rootIndex || !texture) {
			return;
		}
		texture->Transition(commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		commandList->SetGraphicsRootDescriptorTable(*rootIndex, texture->GetSRVHandleGPU());
	};

	bindTexture(kInputColorShaderName, &input);
	bindTexture(kSceneColorShaderName, directX_.GetSceneColor());
	bindTexture(kSceneNormalShaderName, directX_.GetGBufferNormal());
	bindTexture(kSceneAlbedoShaderName, directX_.GetGBufferAlbedo());
	bindTexture(kSceneMaterialShaderName, directX_.GetGBufferMaterial());

	if (const auto rootIndex = pipeline.FindRootIndex(kSceneDepthShaderName)) {
		DepthStencil* depth = directX_.GetDepthStencil();
		if (depth) {
			depth->Transition(commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			commandList->SetGraphicsRootDescriptorTable(*rootIndex, depth->GetSRVHandleGPU());
		}
	}
}

void PostProcessSystem::UpdateContext(RenderTexture& input, RenderTexture& output) {
	const float inputWidth = static_cast<float>(input.GetWidth());
	const float inputHeight = static_cast<float>(input.GetHeight());
	const float outputWidth = static_cast<float>(output.GetWidth());
	const float outputHeight = static_cast<float>(output.GetHeight());
	const float sceneWidth = static_cast<float>(directX_.GetRenderWidth());
	const float sceneHeight = static_cast<float>(directX_.GetRenderHeight());
	const PostProcessContextForGPU context{
		.outputSize = { outputWidth, outputHeight },
		.inverseOutputSize = { 1.0f / outputWidth, 1.0f / outputHeight },
		.inputSize = { inputWidth, inputHeight },
		.inverseInputSize = { 1.0f / inputWidth, 1.0f / inputHeight },
		.sceneSize = { sceneWidth, sceneHeight },
		.inverseSceneSize = { 1.0f / sceneWidth, 1.0f / sceneHeight },
		.frameIndex = directX_.GetCurrentFrameContextIndex(),
	};
	std::memcpy(
		contextResources_.GetMappedData(directX_.GetCurrentFrameContextIndex()),
		&context,
		sizeof(context));
}

bool PostProcessSystem::Present(RenderTexture& input) {
	Pipeline* pipeline = FindPipeline(kPresentPipeline);
	if (!pipeline) {
		return false;
	}
	ID3D12GraphicsCommandList* commandList = directX_.GetCommandList();
	input.Transition(commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	directX_.SetViewportAndScissor(directX_.GetRenderWidth(), directX_.GetRenderHeight());
	const D3D12_CPU_DESCRIPTOR_HANDLE rtv = directX_.GetCurrentBackBufferRTVHandle();
	commandList->OMSetRenderTargets(1u, &rtv, false, nullptr);
	commandList->SetPipelineState(pipeline->GetPipelineState());
	commandList->SetGraphicsRootSignature(pipeline->GetRootSignature());
	commandList->SetGraphicsRootDescriptorTable(
		pipeline->GetRootIndex(kInputColorShaderName),
		input.GetSRVHandleGPU());
	directX_.GetCommandList6()->DispatchMesh(1u, 1u, 1u);
	return true;
}

Pipeline* PostProcessSystem::FindPipeline(std::string_view name) const {
	if (name.empty()) {
		return nullptr;
	}
	Pipeline* pipeline = pipelineManager_.GetPipeline(std::string(name));
	if (!pipeline || !pipeline->IsValid()) {
		Logger::Log("Post Process Pipeline was not found: " + std::string(name));
		return nullptr;
	}
	return pipeline;
}
