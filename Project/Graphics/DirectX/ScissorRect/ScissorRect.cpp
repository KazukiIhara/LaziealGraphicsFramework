#include "ScissorRect.h"

#include "Command/Command.h"

using namespace LGF;

ScissorRect::ScissorRect(Command& cmd)
	:cmd_(cmd) {
}

ScissorRect::~ScissorRect() {
}

void ScissorRect::Resize(uint32_t width, uint32_t height) {
	// シザー矩形の設定
	D3D12_RECT scissorRect{
		.left = 0,
		.top = 0,
		.right = static_cast<LONG>(width),
		.bottom = static_cast<LONG>(height)
	};
	cmd_.GetList()->RSSetScissorRects(1, &scissorRect);
}