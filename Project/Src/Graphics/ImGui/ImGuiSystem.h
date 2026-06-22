#pragma once

#include <cstdint>

#include "Graphics/System/IGraphicsSubsystem.h"

namespace LGF {

	class DirectXCore;

	/// <summary>
	/// ImGuiシステム
	/// </summary>
	class ImGuiSystem :public IGraphicsSubsystem {
	public:
		ImGuiSystem(DirectXCore& directX);
		~ImGuiSystem();

		bool Initialize()override;

		void BeginFrame()override;

		void EndFrame()override;

	private:
		DirectXCore& directX_;

		uint32_t imguiSrvIndex_ = 0;
	};

}