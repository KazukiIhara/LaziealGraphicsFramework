#pragma once

// C++
#include <string>

// DirectX
#include <d3d12.h>
#include <dxcapi.h>

#include "ComPtr/ComPtr.h"

namespace LGF {

	/// <summary>
	/// シェーダーコンパイラ
	/// </summary>
	class ShaderCompiler {
	public:
		ShaderCompiler();
		~ShaderCompiler() = default;

		bool Initialize();

		ComPtr<ID3DBlob> Compile(const std::wstring& filePath, const wchar_t* profile);

	private:
		ComPtr<IDxcUtils> dxcUtils_ = nullptr;
		ComPtr<IDxcCompiler3> dxcCompiler_ = nullptr;
		ComPtr<IDxcIncludeHandler> includeHandler_ = nullptr;
	};

}