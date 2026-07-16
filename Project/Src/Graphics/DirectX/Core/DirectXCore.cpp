#include "DirectXCore.h"

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

using namespace LGF;

namespace {
	constexpr char kSceneColorName[] = "SceneColor";
	constexpr char kFullscreenCopyPipelineName[] = "FullscreenCopy";
	constexpr char kSceneColorShaderName[] = "gSceneColor";
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
	depthStencil_ = std::make_unique<DepthStencil>(*resourceFactory_, *dsvManager_);
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

	RenderTextureDesc sceneColorDesc{};
	sceneColorDesc.width = renderWidth_;
	sceneColorDesc.height = renderHeight_;
	sceneColor_ = renderTextureManager_->Create(kSceneColorName, sceneColorDesc);
	if (!sceneColor_) {
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

	// ここから通常のフレームワーク描画はSceneColorへ書き込む。
	// SwapChainのBackBufferは、このフレーム最後のフルスクリーンコピー用に
	// RENDER_TARGET状態のまま保持しておく。
	sceneColor_->Transition(command_->GetList(), D3D12_RESOURCE_STATE_RENDER_TARGET);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = sceneColor_->GetRTVHandleCPU();
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = depthStencil_->GetDSVHandleCPU();
	command_->GetList()->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);

	sceneColor_->Clear(command_->GetList());
	command_->GetList()->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	viewport_->BeginFrame(renderWidth_, renderHeight_);
	scissorRect_->BeginFrame(renderWidth_, renderHeight_);
	return true;
}

void DirectXCore::DrawSceneToSwapChain(PipelineManager& pipelineManager) {
	assert(sceneColor_);

	Pipeline* pipeline = pipelineManager.GetPipeline(kFullscreenCopyPipelineName);
	assert(pipeline);
	assert(pipeline->IsValid());

	ID3D12GraphicsCommandList* commandList = command_->GetList();
	ID3D12GraphicsCommandList6* commandList6 = command_->GetList6();
	assert(commandList6);

	// SceneColorへの描画が終わったので、最後の画面転送用にSRVへ切り替える。
	// SwapChainのBackBufferはBeginFrame時点ですでにRenderTargetになっている。
	sceneColor_->Transition(commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = swapChain_->GetCurrentBackBufferRTVHandle();
	commandList->OMSetRenderTargets(1, &rtvHandle, false, nullptr);

	commandList->SetPipelineState(pipeline->GetPipelineState());
	commandList->SetGraphicsRootSignature(pipeline->GetRootSignature());
	commandList->SetGraphicsRootDescriptorTable(
		pipeline->GetRootIndex(kSceneColorShaderName),
		sceneColor_->GetSRVHandleGPU());

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
	sceneColor_ = nullptr;
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
