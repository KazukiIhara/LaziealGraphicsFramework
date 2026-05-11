#include "Logger.h"

#include <Windows.h>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <sstream>

std::ofstream Logger::logFile_;

void Logger::Initialize() {
	// すでに空いていたら閉じる
	if (logFile_.is_open()) {
		logFile_.close();
	}

	// ログ保存先を取得
	const std::filesystem::path logDirectory = GetLogDirectory();

	// ログフォルダを作成
	std::filesystem::create_directories(logDirectory);

	// ログファイルを開く
	logFile_.open(CreateLogFilePath(logDirectory), std::ios::out | std::ios::trunc);

	// 初期化ログを出力
	Log("[Logger Initialized]");
}

void Logger::Finalize() {
	// 終了ログを出力
	Log("[Logger Finalized]");

	// ログファイルを閉じる
	if (logFile_.is_open()) {
		logFile_.close();
	}
}

void Logger::Log(const std::string& message) {
	// 出力メッセージを生成
	const std::string fullMessage = CreateLogMessage(message);

	// デバッグ出力へ書き込み
	OutputDebugStringA(fullMessage.c_str());
	OutputDebugStringA("\n");

	// ファイルへ書き込み
	if (logFile_.is_open()) {
		logFile_ << fullMessage << std::endl;
	}
}

std::wstring Logger::ConvertString(const std::string& str) {
	// 空文字は空のワイド文字列を返す
	if (str.empty()) {
		return std::wstring();
	}

	// 必要な文字数を取得
	const int sizeNeeded = MultiByteToWideChar(
		CP_UTF8,
		0,
		str.c_str(),
		static_cast<int>(str.size()),
		nullptr,
		0
	);

	// 変換失敗時は空文字列を返す
	if (sizeNeeded == 0) {
		return std::wstring();
	}

	// 変換先を確保
	std::wstring result(sizeNeeded, 0);

	// UTF-8からワイド文字列へ変換
	MultiByteToWideChar(
		CP_UTF8,
		0,
		str.c_str(),
		static_cast<int>(str.size()),
		result.data(),
		sizeNeeded
	);

	return result;
}

std::string Logger::ConvertString(const std::wstring& str) {
	// 空文字は空の文字列を返す
	if (str.empty()) {
		return std::string();
	}

	// 必要な文字数を取得
	const int sizeNeeded = WideCharToMultiByte(
		CP_UTF8,
		0,
		str.c_str(),
		static_cast<int>(str.size()),
		nullptr,
		0,
		nullptr,
		nullptr
	);

	// 変換失敗時は空文字列を返す
	if (sizeNeeded == 0) {
		return std::string();
	}

	// 変換先を確保
	std::string result(sizeNeeded, 0);

	// ワイド文字列からUTF-8へ変換
	WideCharToMultiByte(
		CP_UTF8,
		0,
		str.c_str(),
		static_cast<int>(str.size()),
		result.data(),
		sizeNeeded,
		nullptr,
		nullptr
	);

	return result;
}

bool Logger::IsRunningUnderPIX() {
	// 環境変数の長さを取得
	char buf[2]{};
	size_t len = 0;
	getenv_s(&len, buf, sizeof(buf), "PIX_PROCESS");

	// PIX_PROCESSが存在するか判定
	return len > 0;
}

std::filesystem::path Logger::GetLogDirectory() {
	// デバッガまたはPIX起動を判定
	const bool runningUnderVS = IsDebuggerPresent();
	const bool runningUnderPix = IsRunningUnderPIX();

	// 実行環境に応じて保存先を切り替え
	if (runningUnderVS || runningUnderPix) {
		return "../generated/logs";
	}

	return "logs";
}

std::tm Logger::GetLocalTime() {
	// 現在時刻を取得
	const auto now = std::chrono::system_clock::now();
	const auto timeT = std::chrono::system_clock::to_time_t(now);

	// ローカル時刻へ変換
	std::tm localTime{};
	localtime_s(&localTime, &timeT);

	return localTime;
}

std::string Logger::CreateLogFilePath(const std::filesystem::path& logDirectory) {
	// 現在時刻を取得
	const std::tm localTime = GetLocalTime();

	// ファイル名を組み立て
	std::ostringstream stream;
	stream << logDirectory.string() << "/"
		<< std::put_time(&localTime, "%Y%m%d_%H%M%S")
		<< ".txt";

	return stream.str();
}

std::string Logger::CreateLogMessage(const std::string& message) {
	// 現在時刻を取得
	const std::tm localTime = GetLocalTime();

	// 時刻プレフィックスを組み立て
	std::ostringstream stream;
	stream << "[" << std::put_time(&localTime, "%H:%M:%S") << "] " << message;

	return stream.str();
}