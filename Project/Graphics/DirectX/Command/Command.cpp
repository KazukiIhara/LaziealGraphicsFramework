#include "Command.h"

#include <cassert>

#include "Device/Device.h"

using namespace LGF;

Command::Command(Device& device)
	: device_(device) {
}

bool Command::Initialize() {
	// コマンドキューを生成する
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	HRESULT hr = device_.GetDevice()->CreateCommandQueue(&commandQueueDesc,
		IID_PPV_ARGS(&commandQueue_));
	assert(SUCCEEDED(hr));

	// コマンドアロケータを生成する
	hr = device_.GetDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator_));
	assert(SUCCEEDED(hr));

	// コマンドリストを生成する
	hr = device_.GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator_.Get(), nullptr, IID_PPV_ARGS(&commandList_));
	assert(SUCCEEDED(hr));

	// メッシュシェーダーのサポートされているデバイスの場合
	if (device_.GetDeviceCaps().isMeshShaderSupported) {
		// MeshShader用：ID3D12GraphicsCommandList6 にキャスト
		hr = commandList_->QueryInterface(IID_PPV_ARGS(&commandList6_));
		assert(SUCCEEDED(hr));
	}

}

void Command::KickCommand() {
	// 積んだコマンドをクローズ
	commandList_->Close();
	// GPUにコマンドリストの実行を行わせる
	ComPtr<ID3D12CommandList> commandLists[] = { commandList_.Get() };
	commandQueue_->ExecuteCommandLists(1, commandLists->GetAddressOf());
}

void Command::ResetCommand() {
	// アロケータのリセット
	HRESULT hr = commandAllocator_->Reset();
	assert(SUCCEEDED(hr));
	// コマンドリストのリセット
	hr = commandList_->Reset(commandAllocator_.Get(), nullptr);
	assert(SUCCEEDED(hr));
}

ID3D12CommandQueue* Command::GetQueue() {
	return commandQueue_.Get();
}

ID3D12CommandAllocator* Command::GetAllocator() {
	return commandAllocator_.Get();
}

ID3D12GraphicsCommandList* Command::GetList() {
	return commandList_.Get();
}

ID3D12GraphicsCommandList6* Command::GetList6() {
	return commandList6_.Get();
}
