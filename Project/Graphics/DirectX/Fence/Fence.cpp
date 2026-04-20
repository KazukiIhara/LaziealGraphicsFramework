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

void Fence::WaitGPU() {
}