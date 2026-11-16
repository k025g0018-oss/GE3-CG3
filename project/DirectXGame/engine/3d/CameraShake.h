#pragma once

#include "Matrix4x4.h"
#include "Vector.h"

#include <random>

// 前方宣言
class TuningFile;

///// ----- 画面シェイク ----- /////

/// <summary>
/// 一定時間、View行列をランダムにずらして画面を揺らす
/// (AL2_評価課題2の画面シェイク : 10フレームの間、毎フレーム±15pxずらす を3Dにしたもの)
/// </summary>
class CameraShake {
public:
	///// ----- メンバ関数 ----- /////

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 揺れを始める (揺れている途中なら、最初からやり直す)
	/// </summary>
	void Start();

	/// <summary>
	/// 更新
	/// 揺れている間は、毎フレームずらす量をランダムに決め直す
	/// </summary>
	/// <param name="deltaTime">前のフレームからの経過時間 (秒)</param>
	void Update(float deltaTime);

	/// <summary>
	/// View行列に揺れを加える
	/// </summary>
	/// <param name="viewMatrix">カメラのView行列</param>
	/// <returns>揺れを加えたView行列 (揺れていないときはそのまま)</returns>
	Matrix4x4 ApplyToView(const Matrix4x4& viewMatrix) const;

	/// <summary>
	/// 調整値を、保存・読み込みできるように登録する (shake.項目名)
	/// </summary>
	/// <param name="tuningFile"></param>
	void RegisterTuning(TuningFile& tuningFile);

	///// ----- ===== ----- /////

	///// ----- getter ----- /////

	/// <summary>
	/// 揺れを使うかどうか
	/// </summary>
	bool& GetIsEnabled() {
		return isEnabled_;
	}

	/// <summary>
	/// 揺れる時間 (秒)
	/// </summary>
	float& GetDuration() {
		return duration_;
	}

	/// <summary>
	/// 揺れの強さ (m)
	/// </summary>
	float& GetAmplitude() {
		return amplitude_;
	}

	/// <summary>
	/// 揺れている途中か
	/// </summary>
	bool IsShaking() const {
		return timer_ > 0.0f;
	}

	///// ----- ===== ----- /////

private:
	///// ----- メンバ変数 ----- /////

	/// --- 設定 ---

	// 揺れを使うかどうか
	bool isEnabled_ = true;

	// 揺れる時間 (秒) AL2では10フレーム → 10/60秒
	float duration_ = 10.0f / 60.0f;

	// 揺れの強さ (m) AL2では15px → Z=0の位置で画面の高さ(約4.6m)の15/720 ≒ 0.1m
	float amplitude_ = 0.1f;

	/// --- 状態 ---

	// 揺れ終わるまでの残り時間 (秒)
	float timer_ = 0.0f;

	// 今のフレームでずらす量 (m) 画面の横(X)と縦(Y)
	Vector2 offset_{0.0f, 0.0f};

	// 乱数生成器
	std::mt19937 randomEngine_;

	///// ----- ===== ----- /////
};

///// ----- ===== ----- /////
