#include "TuningFile.h"

#include <algorithm>
#include <fstream>
#include <sstream>

///// ----- 登録 ----- /////

// 小数
void TuningFile::Add(const std::string& name, float* value) { entries_.push_back({.name = name, .floatValue = value}); }

// オン・オフ
void TuningFile::Add(const std::string& name, bool* value) { entries_.push_back({.name = name, .boolValue = value}); }

// 個数
void TuningFile::Add(const std::string& name, uint32_t* value, uint32_t minValue, uint32_t maxValue) {
	entries_.push_back({.name = name, .uintValue = value, .uintMin = minValue, .uintMax = maxValue});
}

// Vector3
void TuningFile::Add(const std::string& name, Vector3* value) {
	Add(name + ".x", &value->x);
	Add(name + ".y", &value->y);
	Add(name + ".z", &value->z);
}

// Vector4
void TuningFile::Add(const std::string& name, Vector4* value) {
	Add(name + ".x", &value->x);
	Add(name + ".y", &value->y);
	Add(name + ".z", &value->z);
	Add(name + ".w", &value->w);
}

///// ----- ===== ----- /////

///// ----- 保存・読み込み ----- /////

// 書き出し
bool TuningFile::Save(const std::string& filePath) {

	// 保存先のフォルダがなければ作る
	const std::filesystem::path parentPath = std::filesystem::path(filePath).parent_path();
	if (!parentPath.empty()) {
		std::error_code errorCode;
		std::filesystem::create_directories(parentPath, errorCode);
	}

	{
		std::ofstream file(filePath);
		if (!file.is_open()) {
			return false;
		}

		file << "# name,value\n";
		for (const auto& [name, value] : ToMap()) {
			file << name << ',' << value << '\n';
		}
	}

	// 自分で保存した変更でホットリロードしないようにする
	RememberWriteTime(filePath);
	return true;
}

// 読み込み
bool TuningFile::Load(const std::string& filePath) {

	std::ifstream file(filePath);
	if (!file.is_open()) {
		return false;
	}

	std::map<std::string, float> values;

	std::string line;
	while (std::getline(file, line)) {

		// 改行コードの \r を取る
		if (!line.empty() && line.back() == '\r') {
			line.pop_back();
		}

		// 空行とコメント行は飛ばす
		if (line.empty() || line[0] == '#') {
			continue;
		}

		// 名前,値 に分ける
		const size_t comma = line.find(',');
		if (comma == std::string::npos) {
			continue;
		}

		try {
			values[line.substr(0, comma)] = std::stof(line.substr(comma + 1));
		} catch (...) {
			// 数字でなければ飛ばす
		}
	}

	FromMap(values);
	RememberWriteTime(filePath);
	return true;
}

// ファイルが書き換えられていたら読み込み直す
bool TuningFile::ReloadIfChanged(const std::string& filePath) {

	std::error_code errorCode;
	const std::filesystem::file_time_type writeTime = std::filesystem::last_write_time(filePath, errorCode);

	// ファイルがない・前回から変わっていないときは何もしない
	if (errorCode || writeTime == lastWriteTime_) {
		return false;
	}

	return Load(filePath);
}

// 今の値を表にする
std::map<std::string, float> TuningFile::ToMap() const {

	std::map<std::string, float> values;

	for (const Entry& entry : entries_) {
		if (entry.floatValue) {
			values[entry.name] = *entry.floatValue;
		} else if (entry.boolValue) {
			values[entry.name] = *entry.boolValue ? 1.0f : 0.0f;
		} else if (entry.uintValue) {
			values[entry.name] = static_cast<float>(*entry.uintValue);
		}
	}

	return values;
}

// 表の値を変数へ戻す
void TuningFile::FromMap(const std::map<std::string, float>& values) {

	for (const Entry& entry : entries_) {

		// 表にない項目は今の値のまま
		const auto it = values.find(entry.name);
		if (it == values.end()) {
			continue;
		}

		if (entry.floatValue) {
			*entry.floatValue = it->second;
		} else if (entry.boolValue) {
			*entry.boolValue = it->second >= 0.5f;
		} else if (entry.uintValue) {
			// 範囲外の数で配列からはみ出さないように収める
			const float clampedValue = std::clamp(it->second, static_cast<float>(entry.uintMin), static_cast<float>(entry.uintMax));
			*entry.uintValue = static_cast<uint32_t>(clampedValue + 0.5f);
		}
	}
}

// ファイルの最終更新時刻を覚えておく
void TuningFile::RememberWriteTime(const std::string& filePath) {

	std::error_code errorCode;
	const std::filesystem::file_time_type writeTime = std::filesystem::last_write_time(filePath, errorCode);
	if (!errorCode) {
		lastWriteTime_ = writeTime;
	}
}

///// ----- ===== ----- /////
