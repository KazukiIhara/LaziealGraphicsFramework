#include <Windows.h>

#include <memory>

#include "Logger/Logger.h"
#include "Framework/Framework.h"


#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

#pragma comment(lib,"winmm.lib")

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	
	// ログ出力ファイル作成
	Logger::Initialize();

	LGF::Framework framework;

	framework.Initialize();

	while (framework.Update()) {

	}

	framework.Finalize();

	// ログ終了
	Logger::Finalize();

	return 0;
}