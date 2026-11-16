#pragma once

#include "Vector.h"

#include <cstdint>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

/// <summary>
/// 調整値の保存・読み込み
/// 「名前」と「変数の場所」をまとめて登録しておき、1つのファイルに 名前,値 の形で書き出す
/// (ファイルにない名前・知らない名前は無視するので、項目を増やしても古いファイルが読める)
/// </summary>
class TuningFile {
public:
	///// ----- 登録 ----- /////

	// 小数を登録する
	void Add(const std::string& name, float* value);
	// オン・オフを登録する
	void Add(const std::string& name, bool* value);
	// 個数を登録する (読み込んだ値は minValue～maxValue に収める)
	void Add(const std::string& name, uint32_t* value, uint32_t minValue, uint32_t maxValue);
	// Vector3 を登録する (名前.x / 名前.y / 名前.z の3つになる)
	void Add(const std::string& name, Vector3* value);
	// Vector4 を登録する (名前.x / 名前.y / 名前.z / 名前.w の4つになる)
	void Add(const std::string& name, Vector4* value);

	///// ----- ===== ----- /////

	///// ----- 保存・読み込み ----- /////

	// ファイルへ書き出す
	bool Save(const std::string& filePath);
	// ファイルから読み込む (なければ false・今の値のまま)
	bool Load(const std::string& filePath);
	// ファイルが前回の保存・読み込みから書き換えられていたら読み込み直す (ホットリロード)
	bool ReloadIfChanged(const std::string& filePath);

	// 今の値を 名前 -> 値 の表にする
	std::map<std::string, float> ToMap() const;
	// 表の値を変数へ戻す
	void FromMap(const std::map<std::string, float>& values);

	///// ----- ===== ----- /////

private:
	// 登録した1項目
	struct Entry {
		std::string name;
		float* floatValue = nullptr;
		bool* boolValue = nullptr;
		uint32_t* uintValue = nullptr;
		uint32_t uintMin = 0;
		uint32_t uintMax = 0;
	};

	// ファイルの最終更新時刻を覚えておく
	void RememberWriteTime(const std::string& filePath);

	// 登録した項目 (登録した順に書き出す)
	std::vector<Entry> entries_;

	// 前回の保存・読み込み時のファイルの更新時刻
	std::filesystem::file_time_type lastWriteTime_{};
};
