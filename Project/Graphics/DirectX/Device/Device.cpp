#include "Device.h"

#include <array>
#include <cassert>
#include <format>

#include "Logger/Logger.h"

using namespace LGF;

Device::Device() {

}

Device::InitializeResult Device::Initialize() {
	// デバッグレイヤーを有効化
	EnableDebugLayer();

	// DXGIファクトリーを作成
	if (!CreateFactory()) {
		return InitializeResult::kFactoryCreateFailed;
	}

	// 使用するアダプタを選択
	if (!SelectAdapter()) {
		return InitializeResult::kAdapterNotFound;
	}

	// デバイスを作成
	if (!CreateDevice()) {
		return InitializeResult::kDeviceCreateFailed;
	}

	// MeshShader対応を確認
	InitializeResult result = CheckMeshShaderSupport();
	if (result != InitializeResult::kSuccess) {
		return result;
	}

	// InfoQueueを設定
	SetupInfoQueue();

	// 初期化完了ログ
	Logger::Log("DirectXDevice Initialize\n");

	return InitializeResult::kSuccess;
}

void Device::EnableDebugLayer() {
#ifdef _DEBUG
	ComPtr<ID3D12Debug1> debugController = nullptr;
	HRESULT hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
	if (SUCCEEDED(hr)) {
		// デバッグレイヤーを有効化
		debugController->EnableDebugLayer();
		// GPUベース検証を有効化
		debugController->SetEnableGPUBasedValidation(TRUE);
	}
#endif // _DEBUG
}

bool Device::CreateFactory() {
	// DXGIファクトリーを生成
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory_));
	if (FAILED(hr)) {
		return false;
	}

	return true;
}

bool Device::SelectAdapter() {
	// 高性能GPU順にアダプタを列挙
	for (UINT i = 0; dxgiFactory_->EnumAdapterByGpuPreference(
		i,
		DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
		IID_PPV_ARGS(&adapter_)) != DXGI_ERROR_NOT_FOUND; ++i) {

		// アダプタ情報を取得
		DXGI_ADAPTER_DESC3 adapterDesc{};
		HRESULT hr = adapter_->GetDesc3(&adapterDesc);
		if (FAILED(hr)) {
			adapter_ = nullptr;
			continue;
		}

		// ソフトウェアアダプタは除外
		if (adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE) {
			adapter_ = nullptr;
			continue;
		}

		// 採用したアダプタをログ出力
		Logger::Log(Logger::ConvertString(
			std::format(L"Use Adapter : {}\n", adapterDesc.Description)));

		return true;
	}

	return false;
}

bool Device::CreateDevice() {
	// 機能レベル候補
	const std::array<D3D_FEATURE_LEVEL, 3> featureLevels = {
		D3D_FEATURE_LEVEL_12_2,
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
	};

	// 機能レベル文字列
	const std::array<const char*, 3> featureLevelStrings = {
		"12.2",
		"12.1",
		"12.0",
	};

	// 高い機能レベルから順に試す
	for (size_t i = 0; i < featureLevels.size(); ++i) {
		HRESULT hr = D3D12CreateDevice(
			adapter_.Get(),
			featureLevels[i],
			IID_PPV_ARGS(&device_));

		// 作成成功ならログを出す
		if (SUCCEEDED(hr)) {
			Logger::Log(std::format(
				"Feature Level : {}\n",
				featureLevelStrings[i]));
			return true;
		}
	}

	return false;
}

Device::InitializeResult Device::CheckMeshShaderSupport() {
	// MeshShader対応状況を取得
	D3D12_FEATURE_DATA_D3D12_OPTIONS7 options7{};
	HRESULT hr = device_->CheckFeatureSupport(
		D3D12_FEATURE_D3D12_OPTIONS7,
		&options7,
		sizeof(options7));

	if (FAILED(hr)) {
		Logger::Log("Failed to get D3D12_OPTIONS7.\n");
		return InitializeResult::kMeshShaderNotSupported;
	}

	// MeshShaderTierを確認
	if (options7.MeshShaderTier < D3D12_MESH_SHADER_TIER_1) {
		Logger::Log("MeshShaderTier is not supported.\n");
		return InitializeResult::kMeshShaderNotSupported;
	}

	Logger::Log("MeshShaderTier1 or higher is supported.\n");

	// Device10を取得
	hr = device_->QueryInterface(IID_PPV_ARGS(&device10_));
	if (FAILED(hr)) {
		device10_ = nullptr;
		Logger::Log("Failed to get ID3D12Device10.\n");
		return InitializeResult::kDevice10NotAvailable;
	}

	Logger::Log("Device10 acquired.\n");

	return InitializeResult::kSuccess;
}

void Device::SetupInfoQueue() {
#ifdef _DEBUG
	ComPtr<ID3D12InfoQueue> infoQueue = nullptr;
	HRESULT hr = device_->QueryInterface(IID_PPV_ARGS(&infoQueue));
	if (FAILED(hr)) {
		return;
	}

	// 致命的破損で停止
	infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
	// エラーで停止
	infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
	// 警告で停止
	infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

	// 抑制するメッセージID
	D3D12_MESSAGE_ID denyIds[] = {
		D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
	};

	// 抑制する重大度
	D3D12_MESSAGE_SEVERITY severities[] = {
		D3D12_MESSAGE_SEVERITY_INFO
	};

	// フィルタを設定
	D3D12_INFO_QUEUE_FILTER filter{};
	filter.DenyList.NumIDs = _countof(denyIds);
	filter.DenyList.pIDList = denyIds;
	filter.DenyList.NumSeverities = _countof(severities);
	filter.DenyList.pSeverityList = severities;

	// メッセージを抑制
	infoQueue->PushStorageFilter(&filter);
#endif // _DEBUG
}

ComPtr<ID3D12DescriptorHeap> Device::CreateDescriptorHeap(
	D3D12_DESCRIPTOR_HEAP_TYPE heapType,
	UINT numDescriptors,
	bool shaderVisible) {

	// ディスクリプタヒープ設定
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heapType;
	descriptorHeapDesc.NumDescriptors = numDescriptors;

	// シェーダ可視設定
	if (shaderVisible) {
		descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	}
	else {
		descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	}

	// ディスクリプタヒープを生成
	ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
	HRESULT hr = device_->CreateDescriptorHeap(
		&descriptorHeapDesc,
		IID_PPV_ARGS(&descriptorHeap));
	assert(SUCCEEDED(hr));

	return descriptorHeap;
}

ID3D12Device* Device::GetDevice()const{
	return device_.Get();
}

ID3D12Device10* Device::GetDevice10()const {
	return device10_.Get();
}

IDXGIFactory7* Device::GetFactory()const {
	return dxgiFactory_.Get();
}