#pragma once

#include <cstdint>

namespace LGF {

	class Command;

	/// <summary>
	/// シザー矩形
	/// </summary>
	class ScissorRect {
	public:
		ScissorRect(Command& cmd);
		~ScissorRect();

		void Resize(uint32_t width, uint32_t height);

	private:
		Command& cmd_;
	};

}