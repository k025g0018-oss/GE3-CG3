#pragma once

#include <ostream>
#include <string>

///// ----- Logger ----- /////

namespace Logger {

	// 文字列をファイルとデバッグ出力へ書き込む
	void Log(std::ostream& os, const std::string& message);

	// ワイド文字列をファイルとデバッグ出力へ書き込む
	void Log(std::ostream& os, const std::wstring& message);

}