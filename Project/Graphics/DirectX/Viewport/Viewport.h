#pragma once

#include <cstdint>

namespace LGF {

	class Command;

	class Viewport {
	public:
		Viewport(Command& cmd);
		~Viewport();

		void Resize(uint32_t width, uint32_t height);

	private:
		Command& cmd_;
	};
}