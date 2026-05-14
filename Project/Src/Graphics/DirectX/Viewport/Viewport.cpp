#include "Viewport.h"

#include "Command/Command.h"

using namespace LGF;

Viewport::Viewport(Command& cmd)
	:cmd_(cmd) {
}

Viewport::~Viewport() {
}

void Viewport::BeginFrame(uint32_t width, uint32_t height) {
	Resize(width, height);
}

void Viewport::Resize(uint32_t width, uint32_t height) {
	// ビューポートの設定
	D3D12_VIEWPORT viewport{
		.TopLeftX = 0,
		.TopLeftY = 0,
		.Width = static_cast<float>(width),
		.Height = static_cast<float>(height),
		.MinDepth = 0.0f,
		.MaxDepth = 1.0f
	};
	cmd_.GetList()->RSSetViewports(1, &viewport);
}
