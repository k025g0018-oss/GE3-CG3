#pragma once

#include "InstancingParticle.h"

#include <cstdint>
#include <d3d12.h>
#include <random>
#include <string>
#include <vector>

// 前方宣言
class TuningFile;

///// ----- パーティクルグループの設定 ----- /////

/// <summary>
/// 1種類のパーティクル (形状・テクスチャ・動き方) の設定
/// 数値の範囲は min～max の間でランダムに決める
/// </summary>
struct ParticleGroupSettings {
	// Hierarchyに表示する名前
	std::string name;

	// 形状 (PrimitiveMeshで作る)
	std::vector<VertexData> vertices;

	// 最大数
	uint32_t maxCount = 0;

	// 貼るテクスチャの番号 (TextureManager)
	uint32_t textureIndex = 0;

	// 発生範囲 (m)
	Vector3 spawnMin{0.0f, 0.0f, 0.0f};
	Vector3 spawnMax{0.0f, 0.0f, 0.0f};

	// 発生時の大きさ (倍率)
	float scaleMin = 1.0f;
	float scaleMax = 1.0f;

	// 移動の速さ (m/s) 0なら動かない
	float speedMin = 0.0f;
	float speedMax = 0.0f;
	// trueなら上向き (±spreadAngleの範囲) に飛ばす、falseなら画面に沿った全方向に飛ばす
	bool isUpward = false;
	// 上向きに飛ばすときの広がり (rad)
	float spreadAngle = 0.0f;

	// 加速度 (m/s^2) 重力なら {0, -9.8, 0} のように下向きにする
	Vector3 acceleration{0.0f, 0.0f, 0.0f};
	// 空気抵抗 (1秒あたりに速度が減る割合の目安) 0なら減速しない
	float drag = 0.0f;

	// 回転の速さ (rad/s) 0なら回転しない、向きはランダム
	float angularSpeedMin = 0.0f;
	float angularSpeedMax = 0.0f;
	// trueならランダムな軸で回転する (立体用)、falseならZ軸だけで回転する (板用)
	bool isRotateAllAxes = false;

	// 拡縮の幅 (倍率) 0なら大きさは変化しない
	float scalePulseAmount = 0.0f;
	// 拡縮の速さ (rad/s)
	float scalePulseSpeedMin = 0.0f;
	float scalePulseSpeedMax = 0.0f;

	// 寿命 (s)
	float lifeTimeMin = 1.0f;
	float lifeTimeMax = 1.0f;

	// 寿命のうち、フェードインに使う割合 (0.0ならフェードインしない)
	float fadeInRatio = 0.0f;

	// trueなら寿命がなく、消えずにずっと残る
	bool isImmortal = false;

	// trueなら自動では発生せず、Emitを呼んだときだけ発生する (クリックで出す粒など)
	bool isEmitOnDemand = false;

	// trueなら寿命に合わせて小さくなって消える
	bool isShrinkOverLife = false;

	// trueなら粒ごとに色をランダムにする (falseならtintColor)
	bool isRandomColor = false;
	// 色相が1秒で進む量 (1.0で1秒に1周) 0なら色は変わらない (isRandomColorのときだけ使う)
	float hueShiftSpeed = 0.0f;
	// isRandomColorがfalseのときの粒の色
	Vector4 tintColor{1.0f, 1.0f, 1.0f, 1.0f};

	// 描画に使うBlendMode (isSolidのときはkNoneになる)
	BlendMode blendMode = BlendMode::kAdd;

	// trueなら簡易ライティングで陰影をつける (立体用)
	bool isLightingEnabled = false;

	// trueならDepthを書き込むPSOで描画する (不透明な立体用)
	bool isSolid = false;
};

///// ----- ===== ----- /////

///// ----- パーティクルグループ ----- /////

/// <summary>
/// 設定に合わせて、1種類のパーティクルを発生・移動・消滅させる
/// 描画はInstancingParticleでまとめて行う
/// </summary>
class ParticleGroup {
public:
	///// ----- メンバ関数 ----- /////

	ParticleGroup() = default;
	~ParticleGroup() = default;

	// InstancingParticleを持つため、コピー禁止
	ParticleGroup(const ParticleGroup&) = delete;
	ParticleGroup& operator=(const ParticleGroup&) = delete;

	/// <summary>
	/// 初期化
	/// 設定に合わせてInstancingParticleを作り、最初の粒を画面にちりばめる
	/// </summary>
	/// <param name="device"></param>
	/// <param name="srvDescriptorHeap"></param>
	/// <param name="srvIndex">Instancing用SRVを作る番号</param>
	/// <param name="settings">このグループの設定</param>
	void Initialize(
		ID3D12Device* device,
		const DescriptorHeap& srvDescriptorHeap,
		uint32_t srvIndex,
		const ParticleGroupSettings& settings);

	/// <summary>
	/// 更新
	/// 寿命が尽きた粒を再発生させ、移動・回転・拡縮を進めてGPUへ送る
	/// </summary>
	/// <param name="viewMatrix"></param>
	/// <param name="projectionMatrix"></param>
	/// <param name="deltaTime">前のフレームからの経過時間 (秒)</param>
	void Update(const Matrix4x4& viewMatrix, const Matrix4x4& projectionMatrix, float deltaTime);

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="commandList"></param>
	/// <param name="textureHandle">設定のテクスチャ番号のSRV</param>
	void Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle) const;

	/// <summary>
	/// 終了
	/// </summary>
	void Finalize();

	/// <summary>
	/// 全ての粒を発生させ直す (位置・向き・色などを再抽選する)
	/// isEmitOnDemandのときは、全ての粒を消す
	/// </summary>
	void Randomize();

	/// <summary>
	/// 指定した位置から粒を発生させる (空いている粒を使う)
	/// </summary>
	/// <param name="worldPosition">発生させる位置 (ワールド座標、m)</param>
	/// <param name="count">発生させる数</param>
	void Emit(const Vector3& worldPosition, uint32_t count);

	/// <summary>
	/// 数やランダムの範囲などの調整値を、保存・読み込みできるように登録する
	/// </summary>
	/// <param name="tuningFile"></param>
	/// <param name="prefix">ファイルに書く名前の頭 (例: effect.Star)</param>
	void RegisterTuning(TuningFile& tuningFile, const std::string& prefix);

	///// ----- ===== ----- /////

	///// ----- getter ----- /////

	/// <summary>
	/// Hierarchyに表示する名前を取得する
	/// </summary>
	/// <returns></returns>
	const std::string& GetName() const {
		return settings_.name;
	}

	/// <summary>
	/// 貼るテクスチャの番号を取得する
	/// </summary>
	/// <returns></returns>
	uint32_t GetTextureIndex() const {
		return settings_.textureIndex;
	}

	/// <summary>
	/// Depthを書き込むPSOで描画するかどうかを取得する
	/// </summary>
	/// <returns></returns>
	bool IsSolid() const {
		return settings_.isSolid;
	}

	/// <summary>
	/// Propertiesから設定を操作する
	/// </summary>
	/// <returns></returns>
	ParticleGroupSettings& GetSettings() {
		return settings_;
	}

	/// <summary>
	/// 描画を担当するInstancingParticleを取得する (Propertiesから数や色を操作する)
	/// </summary>
	/// <returns></returns>
	InstancingParticle& GetInstancingParticle() {
		return instancingParticle_;
	}

	/// <summary>
	/// 描画に使うBlendModeを取得する
	/// </summary>
	/// <returns></returns>
	BlendMode GetBlendMode() const {
		return blendMode_;
	}

	/// <summary>
	/// Propertiesから描画に使うBlendModeを操作する
	/// </summary>
	/// <returns></returns>
	BlendMode& GetBlendMode() {
		return blendMode_;
	}

	///// ----- ===== ----- /////

private:
	///// ----- メンバ関数 ----- /////

	/// <summary>
	/// 設定に合わせて、新しい粒を1つ発生させる
	/// </summary>
	/// <param name="isFirstSpawn">trueなら経過時間もランダムにして、最初から画面にちりばめる</param>
	/// <returns>発生させた粒</returns>
	Particle MakeNewParticle(bool isFirstSpawn);

	/// <summary>
	/// min～maxの間でランダムな値を返す
	/// </summary>
	/// <param name="min"></param>
	/// <param name="max"></param>
	/// <returns></returns>
	float RandomRange(float min, float max);

	/// <summary>
	/// 色相(0.0～1.0)から、鮮やかな色を作る (HSVのHだけ変えて、S・Vは固定)
	/// </summary>
	/// <param name="hue"></param>
	/// <returns></returns>
	static Vector4 HueToColor(float hue);

	///// ----- ===== ----- /////

	///// ----- メンバ変数 ----- /////

	// このグループの設定
	ParticleGroupSettings settings_;

	// 描画を担当するInstancingParticle
	InstancingParticle instancingParticle_;

	// 描画に使うBlendMode (設定から決める)
	BlendMode blendMode_ = BlendMode::kAdd;

	// 乱数生成器
	std::mt19937 randomEngine_;

	///// ----- ===== ----- /////
};

///// ----- ===== ----- /////
