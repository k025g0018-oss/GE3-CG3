#include "FrameRateController.h"

#include <thread>

using namespace std::chrono;

/// <summary>
/// 初期化
/// </summary>
void FrameRateController::Initialize() {

	// 現在時間を記録する
	reference_ = Clock::now();
}

/// <summary>
/// 更新
/// </summary>
void FrameRateController::Update() {

	/// --- 1_現在時間を取得 ---
	// VSync待ちが完了した時点での時間を取得する
	Clock::time_point now = Clock::now();

	/// --- 2_前回記録からの経過時間を取得する ---
	// 現在時間から前回の時間を引くことで、前回からの経過時間を計算する
	microseconds elapsed = duration_cast<microseconds>(now - reference_);

	/// --- 3_1/60秒(よりわずかに短い時間)たっていない場合 ---
	// 今回の肝、前回から1/60秒たつまで待機する
	// kMinCheckTimeで判定するので、60Hzちょい上のモニターはVSync後に待たずにスルーする
	if (elapsed < kMinCheckTime) {
		// 1/60秒経過するまで微小なスリープを繰り返す
		while (Clock::now() - reference_ < kMinTime) {
			// 1マイクロ秒スリープ
			std::this_thread::sleep_for(microseconds(1));
		}
	}

	/// --- 4_現在の時間を記録する ---
	// 次フレームの2_の計算に使うため、待機完了後の時間を記録しておく
	// これらの処理によって、毎フレームの4_の間隔がぴったり1/60秒に固定される
	// (ただし、処理落ち・描画落ちしている場合はこの限りではない)
	reference_ = Clock::now();
}