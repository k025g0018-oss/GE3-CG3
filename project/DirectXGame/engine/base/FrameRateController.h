#pragma once

#include <chrono>
#include <cstdint>

/// <summary>
/// フレームレートの管理 (FPS固定)
/// </summary>
class FrameRateController {
public:
	///// ----- メンバ関数 ----- /////

	/// <summary>
	/// 初期化 (FPS固定の基準になる時間を記録する)
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新 (前回から1/60秒たつまで待つ。Presentの後に毎フレーム呼ぶ)
	/// </summary>
	void Update();

	///// ----- ===== ----- /////

	///// ----- getter ----- /////

	/// <summary>
	/// 前のフレームからの経過時間 (秒)
	/// </summary>
	float GetDeltaTime() const {
		return deltaTime_;
	}

	/// <summary>
	/// FPSを60に固定しているか
	/// </summary>
	bool IsFixFPS() const {
		return isFixFPS_;
	}

	///// ----- ===== ----- /////

	///// ----- setter ----- /////

	/// <summary>
	/// FPSを60に固定するかどうか (falseならVSyncのみ)
	/// </summary>
	void SetFixFPS(bool isFixFPS) {
		isFixFPS_ = isFixFPS;
	}

	///// ----- ===== ----- /////

private:
	///// ----- 型の別名 ----- /////

	// 長いので短い名前で使えるようにする (ヘッダーなのでusing namespaceは使わない)
	using Clock = std::chrono::steady_clock;

	///// ----- ===== ----- /////

	///// ----- 定数 ----- /////

	// 1/60秒ぴったりの時間
	static constexpr std::chrono::microseconds kMinTime{uint64_t(1000000.0f / 60.0f)};

	// 1/60秒よりわずかに短い時間 (60Hzより少し速いだけのモニターはVSyncだけに任せる)
	static constexpr std::chrono::microseconds kMinCheckTime{uint64_t(1000000.0f / 65.0f)};

	// deltaTimeの上限 (ブレークポイントやウィンドウ操作で止まったときに、大きくワープしないようにする)
	static constexpr float kMaxDeltaTime = 0.1f;

	///// ----- ===== ----- /////

	///// ----- メンバ変数 ----- /////

	// 記録時間 (前回のフレームの時間)
	Clock::time_point reference_;

	// 前のフレームからの経過時間 (秒) 最初のフレームは1/60秒としておく
	float deltaTime_ = 1.0f / 60.0f;

	// FPSを60に固定するか
	bool isFixFPS_ = true;

	///// ----- ===== ----- /////
};