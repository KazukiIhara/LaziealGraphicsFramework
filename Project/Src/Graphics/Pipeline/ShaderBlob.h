#pragma once

#include <string>
#include <d3dcompiler.h>
#include "ComPtr/ComPtr.h"

namespace LGF {
	/// <summary>
	/// シェーダーデータの塊
	/// </summary>
	struct ShaderBlob {
		std::string stageName;
		ComPtr<ID3DBlob> blob;
	};
}