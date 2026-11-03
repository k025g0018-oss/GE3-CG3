#include "StringUtility.h"

#include <Windows.h>

///// ----- 文字コードユーティリティ ----- /////

namespace StringUtility {

	// UTF-8文字列(string)をワイド文字列(wstring)に変換する
	std::wstring ConvertString(const std::string& str) {
		// 空の文字列はそのまま返す
		if (str.empty()) {
			return std::wstring();
		}

		// 変換後に必要な文字数を調べる
		auto sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), NULL, 0);
		if (sizeNeeded == 0) {
			return std::wstring();
		}

		// 必要な分だけ確保して変換する
		std::wstring result(sizeNeeded, 0);
		MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), &result[0], sizeNeeded);
		return result;
	}

	// ワイド文字列(wstring)をUTF-8文字列(string)に変換する
	std::string ConvertString(const std::wstring& str) {
		// 空の文字列はそのまま返す
		if (str.empty()) {
			return std::string();
		}

		// 変換後に必要な文字数を調べる
		auto sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0, NULL, NULL);
		if (sizeNeeded == 0) {
			return std::string();
		}

		// 必要な分だけ確保して変換する
		std::string result(sizeNeeded, 0);
		WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), sizeNeeded, NULL, NULL);
		return result;
	}

}