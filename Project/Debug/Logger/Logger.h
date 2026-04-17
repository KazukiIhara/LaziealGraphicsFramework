#pragma once

#include <ctime>
#include <filesystem>
#include <fstream>
#include <string>

/// <summary>
/// ログ出力クラス
/// </summary>
class Logger {
public:
	// 初期化
	static void Initialize();
	// 終了
	static void Finalize();

	// デバッグログの出力
	static void Log(const std::string& message);
	// string型をwstring型に変換する関数
	static std::wstring ConvertString(const std::string& str);
	// wstring型をstring型に変換する関数
	static std::string ConvertString(const std::wstring& str);

private:
	// PIXから実行されているかどうか
	static bool IsRunningUnderPIX();
	// ログ保存先を取得
	static std::filesystem::path GetLogDirectory();
	// 現在のローカル時刻を取得
	static std::tm GetLocalTime();
	// ログファイル名を生成
	static std::string CreateLogFilePath(const std::filesystem::path& logDirectory);
	// 時刻付きログ文字列を生成
	static std::string CreateLogMessage(const std::string& message);

private:
	static std::ofstream logFile_;
};