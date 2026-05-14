#include "Fence.h"

#include <cassert>

#include "Device/Device.h"
#include "Command/Command.h"

using namespace LGF;

Fence::Fence(Device& device, Command& cmd)
	:device_(device), cmd_(cmd) {
}

Fence::~Fence() {
}

bool Fence::Initialize() {
	// フェンスバリュー初期化
	fenceValue_ = 0;

	// 初期値0でフェンスを作る
	HRESULT hr = device_.GetDevice()->CreateFence(fenceValue_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
	if (FAILED(hr)) {
		assert(false);
		return false;
	}

	return true;
}

void Fence::EndFrame() {
	WaitGPU();
}

void Fence::WaitGPU() {
	// Fenceの値を更新し、GPUGPUがここまでたどり着いたときにFenceの値を指定した値に代入するようにSignalを送る
	cmd_.GetQueue()->Signal(fence_.Get(), ++fenceValue_);
	// Fenceの値が指定されたSignal値にたどり着いているか確認する
	// GetCompletedValueの初期値はFence作成時に渡した初期値
	if (fence_->GetCompletedValue() < fenceValue_) {
		// FenceのSignalを待つためのイベントを作成する
		HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		// 指定したSignalにたどり着いていないので、たどり着くまで待つようにイベントを設定する
		fence_->SetEventOnCompletion(fenceValue_, fenceEvent);
		// イベントを待つ
		WaitForSingleObject(fenceEvent, INFINITE);
		CloseHandle(fenceEvent);
	}
}
