#pragma once

namespace LGF {
	class Window;
	class Device;
	class Command;
	class RTVManager;
	class Viewport;
	class ScissorRect;

	class SwapChain {
	public:
		SwapChain();
		~SwapChain();

	private:
		void CreateSwapChain();
		void CreateResources();
		void CreateRTV();
	};

	SwapChain::SwapChain() {
	}

	SwapChain::~SwapChain() {
	}

}