#include "CameraShake.h"
#include "TuningFile.h"

/// <summary>
/// 初期化
/// </summary>
void CameraShake::Initialize() {

	// 乱数生成器をランダムなシードで初期化する
	std::random_device seedGenerator;
	randomEngine_.seed(seedGenerator());

	timer_ = 0.0f;
	offset_ = {0.0f, 0.0f};
}

/// <summary>
/// 揺れを始める
/// </summary>
void CameraShake::Start() {

	// 揺れを使わない設定なら何もしない
	if (!isEnabled_) {
		return;
	}

	// 残り時間を最初に戻す (AL2の shakeTimer = 10 と同じ)
	timer_ = duration_;
}

/// <summary>
/// 更新
/// </summary>
/// <param name="deltaTime"></param>
void CameraShake::Update(float deltaTime) {

	// 揺れていないときは、ずらさない
	if (timer_ <= 0.0f) {
		offset_ = {0.0f, 0.0f};
		return;
	}

	// -amplitude～+amplitude の間で、横と縦のずらす量をランダムに決める
	// (AL2の rand() % (amplitude * 2 + 1) - amplitude と同じ考え方)
	std::uniform_real_distribution<float> distribution(-amplitude_, amplitude_);
	offset_ = {distribution(randomEngine_), distribution(randomEngine_)};

	// 残り時間を減らす (AL2の shakeTimer-- を秒にしたもの)
	timer_ -= deltaTime;
}

/// <summary>
/// View行列に揺れを加える
/// </summary>
/// <param name="viewMatrix"></param>
/// <returns></returns>
Matrix4x4 CameraShake::ApplyToView(const Matrix4x4& viewMatrix) const {

	// カメラから見た空間 (View空間) で横と縦に平行移動させる
	// → 画面に映る全ての物が同じだけずれる (AL2で全ての描画位置にshakeOffsetを足していたのと同じ)
	const Matrix4x4 shakeMatrix = Matrix4x4::MakeTranslateMatrix({offset_.x, offset_.y, 0.0f});

	// View行列の後ろに掛ける (ワールド → View空間 → ずらす の順)
	return Matrix4x4::Multiply(viewMatrix, shakeMatrix);
}

/// <summary>
/// 調整値を登録する
/// </summary>
/// <param name="tuningFile"></param>
void CameraShake::RegisterTuning(TuningFile& tuningFile) {

	tuningFile.Add("shake.isEnabled", &isEnabled_);
	tuningFile.Add("shake.duration", &duration_);
	tuningFile.Add("shake.amplitude", &amplitude_);
}
