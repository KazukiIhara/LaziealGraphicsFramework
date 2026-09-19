#include "Fence.h"

#include <cassert>

#include "Graphics/DirectX/Device/Device.h"
#include "Graphics/DirectX/Command/Command.h"

using namespace LGF;

Fence::Fence(Device& device, Command& cmd)
	:device_(device), cmd_(cmd) {
}

Fence::~Fence() {
	if (fenceEvent_) {
		CloseHandle(static_cast<HANDLE>(fenceEvent_));
		fenceEvent_ = nullptr;
	}
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

	fenceEvent_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	return fenceEvent_ != nullptr;
}

uint64_t Fence::Signal() {
	const uint64_t signalValue = ++fenceValue_;
	const HRESULT result = cmd_.GetQueue()->Signal(fence_.Get(), signalValue);
	assert(SUCCEEDED(result));
	return signalValue;
}

void Fence::Wait(uint64_t fenceValue) {
	if (fenceValue == 0u || IsCompleted(fenceValue)) {
		return;
	}
	const HRESULT result = fence_->SetEventOnCompletion(
		fenceValue,
		static_cast<HANDLE>(fenceEvent_));
	assert(SUCCEEDED(result));
	WaitForSingleObject(static_cast<HANDLE>(fenceEvent_), INFINITE);
}

void Fence::WaitIdle() {
	Wait(Signal());
}

bool Fence::IsCompleted(uint64_t fenceValue) const {
	return fence_->GetCompletedValue() >= fenceValue;
}

bool Fence::IsInitialized() const {
	return fence_ && fenceEvent_;
}
