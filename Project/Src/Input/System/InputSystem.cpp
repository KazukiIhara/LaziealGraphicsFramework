#include "InputSystem.h"

#include "Core/Window/System/WindowSystem.h"

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

using namespace LGF;

namespace {
	InputSystem* g_inputSystem = nullptr;

	bool IsKeyPressed(uint8_t keyState) {
		return (keyState & 0x80) != 0;
	}
}

InputSystem::InputSystem() {
}

InputSystem::~InputSystem() = default;

bool InputSystem::Initialize(FrameworkContext& context) {
	g_inputSystem = this;

	if (context.window == nullptr) {
		return false;
	}

	return InitializeKeyboard(context.window->GetWindow().GetHwnd());
}

bool InputSystem::Finalize([[maybe_unused]] FrameworkContext& context) {
	if (keyboard_ != nullptr) {
		keyboard_->Unacquire();
		keyboard_.Reset();
	}

	directInput_.Reset();
	g_inputSystem = nullptr;

	return true;
}

void InputSystem::BeginFrame([[maybe_unused]] FrameworkContext& context) {
	UpdateKeyboard();
}

bool InputSystem::IsTriggered(KeyCode code) const {
	const uint8_t key = ToDirectInputKey(code);
	return IsKeyPressed(currentKeys_[key]) && !IsKeyPressed(previousKeys_[key]);
}

bool InputSystem::IsPressed(KeyCode code) const {
	const uint8_t key = ToDirectInputKey(code);
	return IsKeyPressed(currentKeys_[key]);
}

bool InputSystem::IsReleased(KeyCode code) const {
	const uint8_t key = ToDirectInputKey(code);
	return !IsKeyPressed(currentKeys_[key]) && IsKeyPressed(previousKeys_[key]);
}

uint32_t InputSystem::GetPressedFrames(KeyCode code) const {
	return pressedFrames_[ToDirectInputKey(code)];
}

bool InputSystem::InitializeKeyboard(HWND hwnd) {
	HRESULT hr = DirectInput8Create(
		GetModuleHandle(nullptr),
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		reinterpret_cast<void**>(directInput_.GetAddressOf()),
		nullptr
	);

	if (FAILED(hr)) {
		return false;
	}

	hr = directInput_->CreateDevice(GUID_SysKeyboard, keyboard_.GetAddressOf(), nullptr);
	if (FAILED(hr)) {
		return false;
	}

	hr = keyboard_->SetDataFormat(&c_dfDIKeyboard);
	if (FAILED(hr)) {
		return false;
	}

	hr = keyboard_->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(hr)) {
		return false;
	}

	keyboard_->Acquire();

	return true;
}

void InputSystem::UpdateKeyboard() {
	previousKeys_ = currentKeys_;

	if (keyboard_ == nullptr) {
		currentKeys_.fill(0);
		pressedFrames_.fill(0);
		return;
	}

	HRESULT hr = keyboard_->GetDeviceState(static_cast<DWORD>(currentKeys_.size()), currentKeys_.data());
	if (FAILED(hr)) {
		hr = keyboard_->Acquire();
		while (hr == DIERR_INPUTLOST) {
			hr = keyboard_->Acquire();
		}

		if (FAILED(hr)) {
			currentKeys_.fill(0);
		} else {
			hr = keyboard_->GetDeviceState(static_cast<DWORD>(currentKeys_.size()), currentKeys_.data());
			if (FAILED(hr)) {
				currentKeys_.fill(0);
			}
		}
	}

	for (size_t i = 0; i < currentKeys_.size(); ++i) {
		if (IsKeyPressed(currentKeys_[i])) {
			++pressedFrames_[i];
		} else {
			pressedFrames_[i] = 0;
		}
	}
}

uint8_t InputSystem::ToDirectInputKey(KeyCode code) const {
	switch (code) {
		case KeyCode::Space:
			return DIK_SPACE;
		case KeyCode::Escape:
			return DIK_ESCAPE;
		case KeyCode::Enter:
			return DIK_RETURN;
		case KeyCode::Tab:
			return DIK_TAB;
		case KeyCode::Backspace:
			return DIK_BACK;
		case KeyCode::Left:
			return DIK_LEFT;
		case KeyCode::Right:
			return DIK_RIGHT;
		case KeyCode::Up:
			return DIK_UP;
		case KeyCode::Down:
			return DIK_DOWN;
		case KeyCode::A:
			return DIK_A;
		case KeyCode::B:
			return DIK_B;
		case KeyCode::C:
			return DIK_C;
		case KeyCode::D:
			return DIK_D;
		case KeyCode::E:
			return DIK_E;
		case KeyCode::F:
			return DIK_F;
		case KeyCode::G:
			return DIK_G;
		case KeyCode::H:
			return DIK_H;
		case KeyCode::I:
			return DIK_I;
		case KeyCode::J:
			return DIK_J;
		case KeyCode::K:
			return DIK_K;
		case KeyCode::L:
			return DIK_L;
		case KeyCode::M:
			return DIK_M;
		case KeyCode::N:
			return DIK_N;
		case KeyCode::O:
			return DIK_O;
		case KeyCode::P:
			return DIK_P;
		case KeyCode::Q:
			return DIK_Q;
		case KeyCode::R:
			return DIK_R;
		case KeyCode::S:
			return DIK_S;
		case KeyCode::T:
			return DIK_T;
		case KeyCode::U:
			return DIK_U;
		case KeyCode::V:
			return DIK_V;
		case KeyCode::W:
			return DIK_W;
		case KeyCode::X:
			return DIK_X;
		case KeyCode::Y:
			return DIK_Y;
		case KeyCode::Z:
			return DIK_Z;
		case KeyCode::Num0:
			return DIK_0;
		case KeyCode::Num1:
			return DIK_1;
		case KeyCode::Num2:
			return DIK_2;
		case KeyCode::Num3:
			return DIK_3;
		case KeyCode::Num4:
			return DIK_4;
		case KeyCode::Num5:
			return DIK_5;
		case KeyCode::Num6:
			return DIK_6;
		case KeyCode::Num7:
			return DIK_7;
		case KeyCode::Num8:
			return DIK_8;
		case KeyCode::Num9:
			return DIK_9;
		default:
			return DIK_ESCAPE;
	}
}

bool LGF::Input::Internal::IsTriggered(KeyCode code) {
	return g_inputSystem != nullptr && g_inputSystem->IsTriggered(code);
}

bool LGF::Input::Internal::IsPressed(KeyCode code) {
	return g_inputSystem != nullptr && g_inputSystem->IsPressed(code);
}

bool LGF::Input::Internal::IsReleased(KeyCode code) {
	return g_inputSystem != nullptr && g_inputSystem->IsReleased(code);
}

uint32_t LGF::Input::Internal::GetPressedFrames(KeyCode code) {
	if (g_inputSystem == nullptr) {
		return 0;
	}

	return g_inputSystem->GetPressedFrames(code);
}
