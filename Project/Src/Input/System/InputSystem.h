#pragma once

#include <array>
#include <cstdint>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <wrl.h>

#include "Core/Framework/System/IFrameworkSystem.h"
#include "LGF/Input.h"

namespace LGF {

	/// <summary>
	/// キー入力システム
	/// </summary>
	class InputSystem :public IFrameworkSystem {
	public:
		InputSystem();
		~InputSystem()override;

		bool Initialize(FrameworkContext& context)override;
		bool Finalize(FrameworkContext& context)override;

		void BeginFrame(FrameworkContext& context)override;

		bool IsTriggered(KeyCode code)const;
		bool IsPressed(KeyCode code)const;
		bool IsReleased(KeyCode code)const;
		uint32_t GetPressedFrames(KeyCode code)const;

	private:
		bool InitializeKeyboard(HWND hwnd);
		void UpdateKeyboard();
		uint8_t ToDirectInputKey(KeyCode code)const;

	private:
		Microsoft::WRL::ComPtr<IDirectInput8> directInput_;
		Microsoft::WRL::ComPtr<IDirectInputDevice8> keyboard_;
		std::array<uint8_t, 256> currentKeys_{};
		std::array<uint8_t, 256> previousKeys_{};
		std::array<uint32_t, 256> pressedFrames_{};
	};

}
