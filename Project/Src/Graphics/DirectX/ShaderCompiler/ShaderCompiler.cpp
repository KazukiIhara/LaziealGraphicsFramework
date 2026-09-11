#include "ShaderCompiler.h"

#include <cassert>
#include <format>
#include <stdexcept>
#include <vector>

#include "Core/Logger/Logger.h"

using namespace LGF;

ShaderCompiler::ShaderCompiler() {
}

bool ShaderCompiler::Initialize() {
	HRESULT hr = S_FALSE;
	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils_));
	if (FAILED(hr)) {
		return false;
	}
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler_));
	if (FAILED(hr)) {
		return false;
	}
	hr = dxcUtils_->CreateDefaultIncludeHandler(&includeHandler_);
	if (FAILED(hr)) {
		return false;
	}

	return true;
}

ComPtr<ID3DBlob> ShaderCompiler::Compile(const std::wstring& filePath, const wchar_t* profile) {
	// ログ出力
	Logger::Log(Logger::ConvertString(std::format(L"Begin CompileShader, path:{}, profile:{}\n", filePath, profile)));

	// HLSLファイルの読み込み
	ComPtr<IDxcBlobEncoding> shaderSource = nullptr;
	HRESULT hr = dxcUtils_->LoadFile(filePath.c_str(), nullptr, &shaderSource);
	if (FAILED(hr) || !shaderSource) {
		throw std::runtime_error(std::format(
			"Failed to load shader '{}' (0x{:08X}).",
			Logger::ConvertString(filePath),
			static_cast<uint32_t>(hr)));
	}

	DxcBuffer shaderSourceBuffer{};
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;

	// コンパイル引数の指定（Shader Model 6.6 + HLSL2021 + Bindless 対応）
	std::vector<LPCWSTR> arguments = {
		filePath.c_str(),               // 入力ファイル
		L"-E", L"main",					// エントリポイント
		L"-T", profile,                 // ターゲットプロファイル（例: vs_6_6）
		L"-HV", L"2021",                // HLSL言語バージョン（2021=SM6.6）
		L"-Zi", L"-Qembed_debug",       // デバッグ情報
		L"-Zpr",                        // 行優先メモリレイアウト
		L"-D", L"USE_BINDLESS_TEXTURE", // 必要に応じて define 追加
	};

	// 実際のコンパイル処理
	ComPtr<IDxcResult> shaderResult = nullptr;
	hr = dxcCompiler_->Compile(
		&shaderSourceBuffer,
		arguments.data(),
		static_cast<UINT>(arguments.size()),
		includeHandler_.Get(),
		IID_PPV_ARGS(&shaderResult));
	if (FAILED(hr) || !shaderResult) {
		throw std::runtime_error(std::format(
			"Failed to invoke shader compiler for '{}' (0x{:08X}).",
			Logger::ConvertString(filePath),
			static_cast<uint32_t>(hr)));
	}

	// DXCの診断メッセージをログ出力
	ComPtr<IDxcBlobUtf8> shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError && shaderError->GetStringLength() > 0) {
		Logger::Log(shaderError->GetStringPointer());
	}

	HRESULT compileStatus = E_FAIL;
	hr = shaderResult->GetStatus(&compileStatus);
	if (FAILED(hr) || FAILED(compileStatus)) {
		throw std::runtime_error(std::format(
			"Shader compilation failed for '{}'.",
			Logger::ConvertString(filePath)));
	}

	// バイナリを取得
	ComPtr<ID3DBlob> shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	if (FAILED(hr) || !shaderBlob) {
		throw std::runtime_error(std::format(
			"Shader compiler produced no object for '{}' (0x{:08X}).",
			Logger::ConvertString(filePath),
			static_cast<uint32_t>(hr)));
	}

	// コンパイル終了ログ
	Logger::Log(Logger::ConvertString(std::format(L"Compile Succeeded, path:{}, profile:{}\n", filePath, profile)));

	return shaderBlob;
}

IDxcUtils* LGF::ShaderCompiler::GetUtils() {
	return dxcUtils_.Get();
}
