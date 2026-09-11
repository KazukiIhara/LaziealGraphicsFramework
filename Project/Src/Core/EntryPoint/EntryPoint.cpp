#include <Windows.h>

#include <memory>

#include "Core/Logger/Logger.h"
#include "Core/Framework/Framework.h"


#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")

#pragma comment(lib,"dxcompiler.lib")

#pragma comment(lib,"winmm.lib")

void Main();

namespace LGF::Detail {
	Framework& GetFramework();
}

/// <summary>
/// エントリーポイント
/// </summary>
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	// ログシステムの初期化
	Logger::Initialize();

	// フレームワーク初期化、ループ開始
	auto& framework = LGF::Detail::GetFramework();
	if (framework.Initialize()) {
		Main();
		framework.Finalize();
	}

	// ログ終了
	Logger::Finalize();

	return 0;
}
