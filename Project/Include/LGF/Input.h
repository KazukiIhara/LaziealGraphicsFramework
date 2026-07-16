#pragma once

#include <cstdint>

namespace LGF {

	enum class KeyCode : uint8_t {
		Space,
		Escape,
		Enter,
		Tab,
		Backspace,
		Left,
		Right,
		Up,
		Down,
		A,
		B,
		C,
		D,
		E,
		F,
		G,
		H,
		I,
		J,
		K,
		L,
		M,
		N,
		O,
		P,
		Q,
		R,
		S,
		T,
		U,
		V,
		W,
		X,
		Y,
		Z,
		Num0,
		Num1,
		Num2,
		Num3,
		Num4,
		Num5,
		Num6,
		Num7,
		Num8,
		Num9,
	};

	class KeyButton {
	public:
		constexpr explicit KeyButton(KeyCode code) :
			code_(code) {
		}

		bool Trigger()const;
		bool Press()const;
		bool Release()const;
		uint32_t PressedFrames()const;

	private:
		KeyCode code_;
	};

	namespace Key {
		inline constexpr KeyButton Space{ KeyCode::Space };
		inline constexpr KeyButton Escape{ KeyCode::Escape };
		inline constexpr KeyButton Enter{ KeyCode::Enter };
		inline constexpr KeyButton Tab{ KeyCode::Tab };
		inline constexpr KeyButton Backspace{ KeyCode::Backspace };
		inline constexpr KeyButton Left{ KeyCode::Left };
		inline constexpr KeyButton Right{ KeyCode::Right };
		inline constexpr KeyButton Up{ KeyCode::Up };
		inline constexpr KeyButton Down{ KeyCode::Down };

		inline constexpr KeyButton A{ KeyCode::A };
		inline constexpr KeyButton B{ KeyCode::B };
		inline constexpr KeyButton C{ KeyCode::C };
		inline constexpr KeyButton D{ KeyCode::D };
		inline constexpr KeyButton E{ KeyCode::E };
		inline constexpr KeyButton F{ KeyCode::F };
		inline constexpr KeyButton G{ KeyCode::G };
		inline constexpr KeyButton H{ KeyCode::H };
		inline constexpr KeyButton I{ KeyCode::I };
		inline constexpr KeyButton J{ KeyCode::J };
		inline constexpr KeyButton K{ KeyCode::K };
		inline constexpr KeyButton L{ KeyCode::L };
		inline constexpr KeyButton M{ KeyCode::M };
		inline constexpr KeyButton N{ KeyCode::N };
		inline constexpr KeyButton O{ KeyCode::O };
		inline constexpr KeyButton P{ KeyCode::P };
		inline constexpr KeyButton Q{ KeyCode::Q };
		inline constexpr KeyButton R{ KeyCode::R };
		inline constexpr KeyButton S{ KeyCode::S };
		inline constexpr KeyButton T{ KeyCode::T };
		inline constexpr KeyButton U{ KeyCode::U };
		inline constexpr KeyButton V{ KeyCode::V };
		inline constexpr KeyButton W{ KeyCode::W };
		inline constexpr KeyButton X{ KeyCode::X };
		inline constexpr KeyButton Y{ KeyCode::Y };
		inline constexpr KeyButton Z{ KeyCode::Z };

		inline constexpr KeyButton Num0{ KeyCode::Num0 };
		inline constexpr KeyButton Num1{ KeyCode::Num1 };
		inline constexpr KeyButton Num2{ KeyCode::Num2 };
		inline constexpr KeyButton Num3{ KeyCode::Num3 };
		inline constexpr KeyButton Num4{ KeyCode::Num4 };
		inline constexpr KeyButton Num5{ KeyCode::Num5 };
		inline constexpr KeyButton Num6{ KeyCode::Num6 };
		inline constexpr KeyButton Num7{ KeyCode::Num7 };
		inline constexpr KeyButton Num8{ KeyCode::Num8 };
		inline constexpr KeyButton Num9{ KeyCode::Num9 };
	}

	namespace Input::Internal {
		bool IsTriggered(KeyCode code);
		bool IsPressed(KeyCode code);
		bool IsReleased(KeyCode code);
		uint32_t GetPressedFrames(KeyCode code);
	}

}
