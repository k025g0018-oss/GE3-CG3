#pragma once

#include <string>

///// ----- 文字コードユーティリティ ----- /////

namespace StringUtility {

	// UTF-8文字列(string)をワイド文字列(wstring)に変換する
	std::wstring ConvertString(const std::string& str);

	// ワイド文字列(wstring)をUTF-8文字列(string)に変換する
	std::string ConvertString(const std::wstring& str);

}