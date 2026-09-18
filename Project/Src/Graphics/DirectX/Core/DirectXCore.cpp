#include "DirectXCore.h"

#include <cassert>
#include <cmath>
#include <vector>

#include "Core/Window/Window.h"
#include "Graphics/DirectX/Device/Device.h"
#include "Graphics/DirectX/Command/Command.h"
#include "Graphics/DirectX/DescriptorHeap/DescriptorHeapFactory.h"
#include "Graphics/DirectX/Viewmanagers/RTVManager/RTVManager.h"
#include "Graphics/DirectX/Viewmanagers/DSVManager/DSVManager.h"
#include "Graphics/DirectX/Viewmanagers/SRVUAVManager/SRVUAVManager.h"
#include "Graphics/DirectX/ResourceFactory/ResourceFactory.h"
#include "Graphics/DirectX/Viewport/Viewport.h"
#include "Graphics/DirectX/ScissorRect/ScissorRect.h"
#include "Graphics/DirectX/SwapChain/SwapChain.h"
#include "Graphics/DirectX/DepthStencil/DepthStencil.h"
#include "Graphics/DirectX/Fence/Fence.h"
#include "Graphics/DirectX/ShaderCompiler/ShaderCompiler.h"
#include "Graphics/Pipeline/Manager/PipelineManager.h"
#include "Graphics/Pipeline/Pipeline.h"
#include "Graphics/RenderTexture/RenderTexture.h"
#include "Graphics/RenderTexture/RenderTextureManager.h"
#include "Graphics/Light/System/DirectionalLightSystem.h"
#include "Graphics/Camera/System/Camera3DSystem.h"
#include "Graphics/Skybox/System/SkyboxSystem.h"
#include "Graphics/Shadow/System/ShadowMapSystem.h"

using namespace LGF;

namespace {
	constexpr char kSceneColorName[] = "SceneColor";
	constexpr char kGBufferAlbedoName[] = "GBufferAlbedo";
	constexpr char kGBufferNormalName[] = "GBufferNormal";
	constexpr char kGBufferMaterialName[] = "GBufferMaterial";
	constexpr char kFullscreenToneMappingPipelineName[] = "FullscreenToneMapping";
	constexpr char kFullscreenDepthPipelineName[] = "FullscreenDepth";
	constexpr char kFullscreenLightingPipelineName[] = "FullscreenLighting";
	constexpr char kSceneColorShaderName[] = "gSceneColor";
	constexpr char kDepthTextureShaderName[] = "gDepthTexture";
	constexpr char kGBufferAlbedoShaderName[] = "gGBufferAlbedo";
	constexpr char kGBufferNormalShaderName[] = "gGBufferNormal";
	constexpr char kGBufferMaterialShaderName[] = "gGBufferMaterial";
	constexpr char kDirectionalLightShaderName[] = "DirectionalLightData";
	constexpr char kCameraShaderName[] = "gCamera";
	constexpr char kSkyboxShaderName[] = "SkyboxData";
	constexpr char kShadowMapShaderName[] = "gShadowMap";
	constexpr char kShadowCameraShaderName[] = "gShadowCamera";
	constexpr char kToneMappingShaderName[] = "ToneMappingData";
	constexpr size_t kConstantBufferAlignment = 256u;
}

DirectXCore::DirectXCore(Window& window) :
	window_(window) {
}

DirectXCore::~DirectXCore() = default;

bool DirectXCore::Initialize() {
	renderWidth_ = window_.GetWndSetting().wndSize.width;
	renderHeight_ = window_.GetWndSetting().wndSize.height;

	device_ = std::make_unique<Device>();
	command_ = std::make_unique<Command>(*device_);
	descriptorHeapFactory_ = std::make_unique<DescriptorHeapFactory>(*device_);
	rtvManager_ = std::make_unique<RTVManager>(*device_, *descriptorHeapFactory_);
	dsvManager_ = std::make_unique<DSVManager>(*device_, *descriptorHeapFactory_);
	srvUavManager_ = std::make_unique<SRVUAVManager>(*device_, *descriptorHeapFactory_);
	resourceFactory_ = std::make_unique<ResourceFactory>(*device_);
	viewport_ = std::make_unique<Viewport>(*command_);
	scissorRect_ = std::make_unique<ScissorRect>(*command_);
	swapChain_ = std::make_unique<SwapChain>(window_, *device_, *command_, *rtvManager_, *viewport_, *scissorRect_);
	depthStencil_ = std::make_unique<DepthStencil>(*resourceFactory_, *dsvManager_, *srvUavManager_);
	fence_ = std::make_unique<Fence>(*device_, *command_);
	shaderCompiler_ = std::make_unique<ShaderCompiler>();
	renderTextureManager_ = std::make_unique<RenderTextureManager>(
		*resourceFactory_,
		*rtvManager_,
		*srvUavManager_);

	if (Device::InitializeResult::kSuccess != device_->Initialize()) {
		return false;
	}
	if (!command_->Initialize()) {
		return false;
	}
	rtvManager_->Initialize();
	dsvManager_->Initialize();
	srvUavManager_->Initialize();
	if (!swapChain_->Initialize()) {
		return false;
	}
	depthStencil_->Initialize(renderWidth_, renderHeight_);
	if (!fence_->Initialize()) {
		return false;
	}
	if (!shaderCompiler_->Initialize()) {
		return false;
	}

	toneMappingResource_ = resourceFactory_->CreateUploadBufferResource(kConstantBufferAlignment);
	if (!toneMappingResource_) {
		return false;
	}
	const HRESULT toneMappingMapResult = toneMappingResource_->Map(
		0,
		nullptr,
		reinterpret_cast<void**>(&toneMappingForGPU_));
	if (FAILED(toneMappingMapResult)) {
		return false;
	}
	SetExposure(0.0f);

	RenderTextureDesc sceneColorDesc{};
	sceneColorDesc.width = renderWidth_;
	sceneColorDesc.height = renderHeight_;
	sceneColorDesc.resourceFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
	sceneColorDesc.rtvFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
	sceneColorDesc.srvFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
	sceneColor_ = renderTextureManager_->Create(kSceneColorName, sceneColorDesc);
	if (!sceneColor_) {
		return false;
	}

	RenderTextureDesc gBufferAlbedoDesc{};
	gBufferAlbedoDesc.width = renderWidth_;
	gBufferAlbedoDesc.height = renderHeight_;
	gBufferAlbedo_ = renderTextureManager_->Create(kGBufferAlbedoName, gBufferAlbedoDesc);
	if (!gBufferAlbedo_) {
		return false;
	}

	RenderTextureDesc gBufferNormalDesc{};
	gBufferNormalDesc.width = renderWidth_;
	gBufferNormalDesc.height = renderHeight_;
	gBufferNormalDesc.resourceFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
	gBufferNormalDesc.rtvFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
	gBufferNormalDesc.srvFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
	gBufferNormalDesc.clearColor = { 0.0f, 1.0f, 0.0f, 1.0f };
	gBufferNormal_ = renderTextureManager_->Create(kGBufferNormalName, gBufferNormalDesc);
	if (!gBufferNormal_) {
		return false;
	}

	RenderTextureDesc gBufferMaterialDesc{};
	gBufferMaterialDesc.width = renderWidth_;
	gBufferMaterialDesc.height = renderHeight_;
	gBufferMaterialDesc.resourceFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	gBufferMaterialDesc.rtvFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	gBufferMaterialDesc.srvFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	gBufferMaterialDesc.clearColor = { 1.0f, 1.0f, 0.0f, 1.0f };
	gBufferMaterial_ = renderTextureManager_->Create(kGBufferMaterialName, gBufferMaterialDesc);
	if (!gBufferMaterial_) {
		return false;
	}

	return true;
}

bool DirectXCore::BeginFrame() {
	if (window_.IsResized()) {
		const Window::Setting windowSetting = window_.GetWndSetting();
		if (Resize(windowSetting.wndSize.width, windowSetting.wndSize.height)) {
			window_.ClearResizeFlag();
		} else {
			return false;
		}
	}

	command_->BeginFrame();
	swapChain_->BeginFrame();

	RenderTexture* renderTargets[] = { gBufferAlbedo_, gBufferNormal_, gBufferMaterial_ };
	SetRenderTargets(renderTargets);
	ClearRenderTargets(renderTargets);

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = depthStencil_->GetDSVHandleCPU();
	command_->GetList()->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	viewport_->BeginFrame(renderWidth_, renderHeight_);
	scissorRect_->BeginFrame(renderWidth_, renderHeight_);
	return true;
}

void DirectXCore::SetRenderTargets(std::span<RenderTexture* const> renderTextures, bool useDepthStencil) {
	assert(!renderTextures.empty());

	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvHandles;
	rtvHandles.reserve(renderTextures.size());

	// RenderTextureをRenderTarget状態へ遷移
	for (RenderTexture* renderTexture : renderTextures) {
		assert(renderTexture);
		renderTexture->Transition(command_->GetList(), D3D12_RESOURCE_STATE_RENDER_TARGET);
		rtvHandles.push_back(renderTexture->GetRTVHandleCPU());
	}

	// DepthStencilの設定
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle{};
	D3D12_CPU_DESCRIPTOR_HANDLE* dsvHandlePtr = nullptr;
	if (useDepthStencil) {
		depthStencil_->Transition(command_->GetList(), D3D12_RESOURCE_STATE_DEPTH_WRITE);
		dsvHandle = depthStencil_->GetDSVHandleCPU();
		dsvHandlePtr = &dsvHandle;
	}

	// RenderTarget配列の設定
	command_->GetList()->OMSetRenderTargets(
		static_cast<UINT>(rtvHandles.size()),
		rtvHandles.data(),
		false,
		dsvHandlePtr);
}

void DirectXCore::ClearRenderTargets(std::span<RenderTexture* const> renderTextures) {
	assert(!renderTextures.empty());

	// RenderTexture配列のクリア
	for (RenderTexture* renderTexture : renderTextures) {
		assert(renderTexture);
		renderTexture->Clear(command_->GetList());
	}
}

void DirectXCore::DrawLightingToSceneColor(
	PipelineManager& pipelineManager,
	DirectionalLightSystem& lightSystem,
	Camera3DSystem& cameraSystem,
	SkyboxSystem& skyboxSystem,
	ShadowMapSystem& shadowMapSystem) {
	assert(sceneColor_);
	assert(gBufferAlbedo_);
	assert(gBufferNormal_);
	assert(gBufferMaterial_);

	Pipeline* pipeline = pipelineManager.GetPipeline(kFullscreenLightingPipelineName);
	assert(pipeline);
	assert(pipeline->IsValid());

	ID3D12GraphicsCommandList* commandList = command_->GetList();
	ID3D12GraphicsCommandList6* commandList6 = command_->GetList6();
	assert(commandList6);

	// GBufferをShaderResource状態へ遷移
	gBufferAlbedo_->Transition(commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	gBufferNormal_->Transition(commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	gBufferMaterial_->Transition(commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	depthStencil_->Transition(commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	// SceneColorをRenderTarget状態へ遷移
	sceneColor_->Transition(commandList, D3D12_RESOURCE_STATE_RENDER_TARGET);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = sceneColor_->GetRTVHandleCPU();
	commandList->OMSetRenderTargets(1, &rtvHandle, false, nullptr);

	commandList->SetPipelineState(pipeline->GetPipelineState());
	commandList->SetGraphicsRootSignature(pipeline->GetRootSignature());
	commandList->SetGraphicsRootDescriptorTable(
		pipeline->GetRootIndex(kGBufferAlbedoShaderName),
		gBufferAlbedo_->GetSRVHandleGPU());
	commandList->SetGraphicsRootDescriptorTable(
		pipeline->GetRootIndex(kGBufferNormalShaderName),
		gBufferNormal_->GetSRVHandleGPU());
	commandList->SetGraphicsRootDescriptorTable(
		pipeline->GetRootIndex(kGBufferMaterialShaderName),
		gBufferMaterial_->GetSRVHandleGPU());
	commandList->SetGraphicsRootDescriptorTable(
		pipeline->GetRootIndex(kDepthTextureShaderName),
		depthStencil_->GetSRVHandleGPU());
	lightSystem.Bind(commandList, pipeline->GetRootIndex(kDirectionalLightShaderName));
	cameraSystem.Bind(commandList, pipeline->GetRootIndex(kCameraShaderName));
	skyboxSystem.Bind(commandList, pipeline->GetRootIndex(kSkyboxShaderName));
	commandList->SetGraphicsRootDescriptorTable(
		pipeline->GetRootIndex(kShadowMapShaderName),
		shadowMapSystem.GetShadowMapHandleGPU());
	shadowMapSystem.BindCascadeData(commandList, pipeline->GetRootIndex(kShadowCameraShaderName));

	commandList6->DispatchMesh(1, 1, 1);
}

void DirectXCore::DrawSceneToSwapChain(PipelineManager& pipelineManager) {
	assert(sceneColor_);

	Pipeline* pipeline = pipelineManager.GetPipeline(kFullscreenToneMappingPipelineName);
	assert(pipeline);
	assert(pipeline->IsValid());

	ID3D12GraphicsCommandList* commandList = command_->GetList();
	ID3D12GraphicsCommandList6* commandList6 = command_->GetList6();
	assert(commandList6);

	// SceneColorをShaderResource状態へ遷移
	sceneColor_->Transition(commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = swapChain_->GetCurrentBackBufferRTVHandle();
	commandList->OMSetRenderTargets(1, &rtvHandle, false, nullptr);

	commandList->SetPipelineState(pipeline->GetPipelineState());
	commandList->SetGraphicsRootSignature(pipeline->GetRootSignature());
	commandList->SetGraphicsRootDescriptorTable(
		pipeline->GetRootIndex(kSceneColorShaderName),
		sceneColor_->GetSRVHandleGPU());
	commandList->SetGraphicsRootConstantBufferView(
		pipeline->GetRootIndex(kToneMappingShaderName),
		toneMappingResource_->GetGPUVirtualAddress());

	commandList6->DispatchMesh(1, 1, 1);
}

void DirectXCore::RestoreGBufferRenderTargets() {
	RenderTexture* renderTargets[] = { gBufferAlbedo_, gBufferNormal_, gBufferMaterial_ };
	SetRenderTargets(renderTargets);
	viewport_->BeginFrame(renderWidth_, renderHeight_);
	scissorRect_->BeginFrame(renderWidth_, renderHeight_);
}

void DirectXCore::SetExposure(float exposureEV) {
	if (!toneMappingForGPU_) {
		return;
	}

	toneMappingForGPU_->exposureEV = std::isfinite(exposureEV) ? exposureEV : 0.0f;
}

void DirectXCore::DrawDepthToSwapChain(PipelineManager& pipelineManager) {
	Pipeline* pipeline = pipelineManager.GetPipeline(kFullscreenDepthPipelineName);
	assert(pipeline);
	assert(pipeline->IsValid());

	ID3D12GraphicsCommandList* commandList = command_->GetList();
	ID3D12GraphicsCommandList6* commandList6 = command_->GetList6();
	assert(commandList6);

	// DepthStencilをShaderResource状態へ遷移
	depthStencil_->Transition(commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = swapChain_->GetCurrentBackBufferRTVHandle();
	commandList->OMSetRenderTargets(1, &rtvHandle, false, nullptr);

	commandList->SetPipelineState(pipeline->GetPipelineState());
	commandList->SetGraphicsRootSignature(pipeline->GetRootSignature());
	commandList->SetGraphicsRootDescriptorTable(
		pipeline->GetRootIndex(kDepthTextureShaderName),
		depthStencil_->GetSRVHandleGPU());

	commandList6->DispatchMesh(1, 1, 1);
}

void DirectXCore::EndFrame() {
	swapChain_->EndFrame();
	command_->EndFrame();
	swapChain_->Present();
	fence_->EndFrame();
}

void DirectXCore::Finalize() {
	if (renderTextureManager_) {
		renderTextureManager_->Finalize();
	}
	toneMappingForGPU_ = nullptr;
	toneMappingResource_.Reset();
	sceneColor_ = nullptr;
	gBufferAlbedo_ = nullptr;
	gBufferNormal_ = nullptr;
	gBufferMaterial_ = nullptr;
	renderTextureManager_.reset();
	shaderCompiler_.reset();
	fence_.reset();
	depthStencil_.reset();
	swapChain_.reset();
	scissorRect_.reset();
	viewport_.reset();
	resourceFactory_.reset();
	srvUavManager_.reset();
	dsvManager_.reset();
	rtvManager_.reset();
	descriptorHeapFactory_.reset();
	command_.reset();
	device_.reset();
}

bool DirectXCore::Resize(uint32_t width, uint32_t height) {
	if (width == 0 || height == 0) {
		return false;
	}

	if (width == renderWidth_ && height == renderHeight_) {
		return true;
	}

	fence_->WaitGPU();

	if (!swapChain_->Resize(width, height)) {
		return false;
	}

	depthStencil_->Resize(width, height);
	if (!renderTextureManager_->ResizeAll(width, height)) {
		return false;
	}

	sceneColor_ = renderTextureManager_->Get(kSceneColorName);
	gBufferAlbedo_ = renderTextureManager_->Get(kGBufferAlbedoName);
	gBufferNormal_ = renderTextureManager_->Get(kGBufferNormalName);
	gBufferMaterial_ = renderTextureManager_->Get(kGBufferMaterialName);
	renderWidth_ = width;
	renderHeight_ = height;
	return true;
}

ID3D12Device* DirectXCore::GetDevice() {
	return device_->GetDevice();
}

ID3D12GraphicsCommandList* DirectXCore::GetCommandList() {
	return command_->GetList();
}

ID3D12GraphicsCommandList6* DirectXCore::GetCommandList6() {
	return command_->GetList6();
}

ID3D12CommandQueue* LGF::DirectXCore::GetCommandQueue() {
	return command_->GetQueue();
}

DirectXCore::ResMgr DirectXCore::GetResMgr() {
	return ResMgr(rtvManager_.get(), dsvManager_.get(), srvUavManager_.get());
}

ResourceFactory* DirectXCore::GetResourceFactory() {
	return resourceFactory_.get();
}

ShaderCompiler* DirectXCore::GetShaderCompiler() {
	return shaderCompiler_.get();
}

RenderTextureManager* DirectXCore::GetRenderTextureManager() {
	return renderTextureManager_.get();
}
