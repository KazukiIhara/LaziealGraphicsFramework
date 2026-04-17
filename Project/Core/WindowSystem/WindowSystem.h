#pragma once

#include <cstdint>
#include <string>
#include <Windows.h>

/// <summary>
/// ウィンドウ管理クラス
/// </summary>
class WindowSystem {
public:
	struct WndSize {
		uint32_t width{};
		uint32_t height{};
	};

	struct Setting {
		WndSize wndSize{};
		std::wstring wndName{};
		bool isFullScreen = false;
	};
public:
	WindowSystem(const WindowSystem::Setting& setting);
	~WindowSystem();

	// メッセージの処理
	bool ProcessMessage();

	// ウィンドウハンドルの取得
	HWND GetHwnd() const;
	// ウィンドウクラスの取得
	WNDCLASS GetWndClass() const;
	// ウィンドウサイズの取得
	WindowSystem::Setting GetWndSetting()const;

	// フルスクリーンモード切り替え
	void ToggleFullScreen();

private:
	bool Initialize(const WindowSystem::Setting& setting);
	bool Finalize();

	// ゲームウィンドウの作成
	void CreateGameWindow(int32_t clientWidth = 1280, int32_t clientHeight = 720,
		const std::wstring& windowName = L"LGF", UINT windowStyle = WS_OVERLAPPEDWINDOW);

	// ウィンドウプロシージャ
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

private:
	// ウィンドウハンドル
	HWND hwnd_ = nullptr;
	// ウィンドウクラス
	WNDCLASS wc_{};
	// ウィンドウ設定
	WindowSystem::Setting setting_;
	// ウィンドウモード時の位置とサイズ
	RECT windowRect_{};
	// Comの初期化判別フラグ
	bool isCOMInitialized_ = false;
};