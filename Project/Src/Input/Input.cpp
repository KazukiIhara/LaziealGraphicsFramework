#include "LGF/Input.h"

using namespace LGF;

bool KeyButton::Trigger() const {
	return Input::Internal::IsTriggered(code_);
}

bool KeyButton::Press() const {
	return Input::Internal::IsPressed(code_);
}

bool KeyButton::Release() const {
	return Input::Internal::IsReleased(code_);
}

uint32_t KeyButton::PressedFrames() const {
	return Input::Internal::GetPressedFrames(code_);
}
