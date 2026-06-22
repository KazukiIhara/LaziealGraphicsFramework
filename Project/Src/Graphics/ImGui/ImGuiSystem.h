#pragma once

#include <cstdint>

#include <imgui.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_win32.h>

#include "Graphics/System/IGraphicsSubsystem.h"

namespace LGF {

	class DirectXCore;
	class Window;

	/// <summary>
	/// ImGuiシステム
	/// </summary>
	class ImGuiSystem :public IGraphicsSubsystem {
	public:
		ImGuiSystem(DirectXCore& directX, Window& window);
		~ImGuiSystem()override;

		bool Initialize()override;
		void Finalize()override;

		void BeginFrame()override;

		void EndFrame()override;
	private:
		static void AllocateSrvDescriptor(
			ImGui_ImplDX12_InitInfo* info,
			D3D12_CPU_DESCRIPTOR_HANDLE* outCpuHandle,
			D3D12_GPU_DESCRIPTOR_HANDLE* outGpuHandle
		);

		static void FreeSrvDescriptor(
			ImGui_ImplDX12_InitInfo* info,
			D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
			D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle
		);
	private:
		DirectXCore& directX_;
		Window& window_;

		uint32_t imguiSrvIndex_ = 0;
	};

}