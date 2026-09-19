#include "Command.h"

#include <cassert>

#include "Graphics/DirectX/Device/Device.h"

using namespace LGF;

Command::Command(Device& device)
	: device_(device) {
}

Command::~Command() {
}

bool Command::Initialize() {
	// コマンドキューを作成
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	HRESULT hr = device_.GetDevice()->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue_));
	if (FAILED(hr)) {
		assert(false);
		return false;
	}
	// コマンドアロケータを作成
	for (FrameContext& frameContext : frameContexts_) {
		hr = device_.GetDevice()->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(&frameContext.commandAllocator));
		if (FAILED(hr)) {
			assert(false);
			return false;
		}
	}
	// コマンドリストを作成
	hr = device_.GetDevice()->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		frameContexts_.front().commandAllocator.Get(),
		nullptr,
		IID_PPV_ARGS(&commandList_));
	if (FAILED(hr)) {
		assert(false);
		return false;
	}
	// MeshShader対応デバイスならList6を取得
	if (device_.GetDeviceCaps().isMeshShaderSupported) {
		hr = commandList_->QueryInterface(IID_PPV_ARGS(&commandList6_));
		if (FAILED(hr)) {
			assert(false);
			return false;
		}
	}
	hr = commandList_->Close();
	if (FAILED(hr)) {
		assert(false);
		return false;
	}

	return true;
}

void Command::BeginFrame(uint32_t frameContextIndex) {
	ResetCommand(frameContextIndex);
}

void Command::EndFrame() {
	KickCommand();
}

void Command::KickCommand() {
	// コマンドをクローズ
	commandList_->Close();
	// GPUにコマンドリストの実行を命令
	ID3D12CommandList* commandLists[] = { commandList_.Get() };
	commandQueue_->ExecuteCommandLists(1, commandLists);
}

void Command::ResetCommand(uint32_t frameContextIndex) {
	FrameContext& frameContext = GetFrameContext(frameContextIndex);
	// アロケータのリセット
	HRESULT hr = frameContext.commandAllocator->Reset();
	assert(SUCCEEDED(hr));
	// コマンドリストのリセット
	hr = commandList_->Reset(frameContext.commandAllocator.Get(), nullptr);
	assert(SUCCEEDED(hr));
}

FrameContext& Command::GetFrameContext(uint32_t frameContextIndex) {
	return frameContexts_.at(frameContextIndex);
}

ID3D12CommandQueue* Command::GetQueue() {
	return commandQueue_.Get();
}

ID3D12GraphicsCommandList* Command::GetList() {
	return commandList_.Get();
}

ID3D12GraphicsCommandList6* Command::GetList6() {
	return commandList6_.Get();
}
