#include "Logger.h"
#include "StringUtility.h"

#include <Windows.h>

///// ----- Logger ----- /////

namespace Logger {

	// 文字列をファイルとデバッグ出力へ書き込む
	void Log(std::ostream& os, const std::string& message) {
		os << message << std::endl;
		OutputDebugStringA(message.c_str());
	}

	// ワイド文字列をファイルとデバッグ出力へ書き込む
	void Log(std::ostream& os, const std::wstring& message) {
		std::string str = StringUtility::ConvertString(message);
		os << str << std::endl;
		OutputDebugStringA(str.c_str());
	}

}