#include "ImGuiSystem.h"

#include <imgui.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_win32.h>

using namespace LGF;

ImGuiSystem::ImGuiSystem(DirectXCore& directX) :
	directX_(directX) {
}

ImGuiSystem::~ImGuiSystem() = default;

bool ImGuiSystem::Initialize() {
	// ImGui本体初期化
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	// 設定
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	return true;
}

void ImGuiSystem::BeginFrame() {
}

void ImGuiSystem::EndFrame() {
}
