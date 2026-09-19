#include "ImGuiSystem.h"

#include "Graphics/DirectX/Core/DirectXCore.h"
#include "Graphics/DirectX/ViewManagers/SRVUAVManager/SRVUAVManager.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND hwnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam);

using namespace LGF;

ImGuiSystem::ImGuiSystem(DirectXCore& directX, void* windowHandle) :
	directX_(directX), windowHandle_(windowHandle) {
}

ImGuiSystem::~ImGuiSystem() = default;

bool ImGuiSystem::Initialize() {
	// ImGui本体初期化
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	// 設定
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	// Win32初期化
	ImGui_ImplWin32_Init(static_cast<HWND>(windowHandle_));

	// DX12初期化
	ImGui_ImplDX12_InitInfo initInfo{};
	initInfo.Device = directX_.GetDevice();
	initInfo.CommandQueue = directX_.GetCommandQueue();
	initInfo.NumFramesInFlight = kFrameContextCount;
	initInfo.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	initInfo.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	initInfo.SrvDescriptorHeap = directX_.GetResMgr().srvuav->GetDescriptorHeap();
	initInfo.UserData = directX_.GetResMgr().srvuav;
	initInfo.SrvDescriptorAllocFn = AllocateSrvDescriptor;
	initInfo.SrvDescriptorFreeFn = FreeSrvDescriptor;

	return ImGui_ImplDX12_Init(&initInfo);
}

void ImGuiSystem::Finalize() {
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void ImGuiSystem::BeginFrame() {
	// 新しいフレーム開始
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void ImGuiSystem::EndFrame() {
	// 描画データ確定
	ImGui::Render();

	// ImGui描画
	ImGui_ImplDX12_RenderDrawData(
		ImGui::GetDrawData(),
		directX_.GetCommandList()
	);
}

void ImGuiSystem::AllocateSrvDescriptor(
	ImGui_ImplDX12_InitInfo* info,
	D3D12_CPU_DESCRIPTOR_HANDLE* outCpuHandle,
	D3D12_GPU_DESCRIPTOR_HANDLE* outGpuHandle) {

	// SRVUAVManagerを取得
	auto* srvuav = static_cast<SRVUAVManager*>(info->UserData);

	// ImGui用SRVを確保
	const uint32_t index = srvuav->Allocate();

	// CPU/GPUハンドルを返す
	*outCpuHandle = srvuav->GetCPUDescriptorHandle(index);
	*outGpuHandle = srvuav->GetGPUDescriptorHandle(index);
}

void ImGuiSystem::FreeSrvDescriptor(
	ImGui_ImplDX12_InitInfo* info,
	[[maybe_unused]] D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
	[[maybe_unused]] D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle) {

	// 現状は解放処理なし
	auto* srvuav = static_cast<SRVUAVManager*>(info->UserData);
	(void)srvuav;
}

bool ImGuiSystem::HandleWindowMessage(
	void* windowHandle,
	uint32_t message,
	uintptr_t wParam,
	intptr_t lParam) {
	return ImGui_ImplWin32_WndProcHandler(
		static_cast<HWND>(windowHandle),
		message,
		static_cast<WPARAM>(wParam),
		static_cast<LPARAM>(lParam)) != 0;
}
