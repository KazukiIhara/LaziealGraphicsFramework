#include <Windows.h>

#include <memory>

#include "Logger/Logger.h"
#include "Framework/Framework.h"


#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

#pragma comment(lib,"dxcompiler.lib")

#pragma comment(lib,"winmm.lib")

void Main();

namespace LGF::Detail {
	Framework& GetFramework();
}


int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    Logger::Initialize();

    auto& framework = LGF::Detail::GetFramework();

    if (framework.Initialize()) {
        Main();
        framework.Finalize();
    }

    Logger::Finalize();

    return 0;
}