#pragma once

// ウィンドウ
#include "Window/Window.h"

/// <summary>
/// フレームワーク
/// </summary>
class Framework {
public:
	struct Context {

	};
public:
	Framework();
	~Framework();

	bool Initialize();
	void Finalize();

	void Run();

private:

	void BeginFrame();
	void EndFrame();

private:

};