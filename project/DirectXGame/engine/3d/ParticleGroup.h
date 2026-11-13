#pragma once

#include "InstancingParticle.h"

#include <cstdint>
#include <d3d12.h>
#include <random>
#include <string>
#include <vector>

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

	// Z軸回転の速さ (rad/s) 0なら回転しない、向きはランダム
	float angularSpeedMin = 0.0f;
	float angularSpeedMax = 0.0f;

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

	///// ----- ===== ----- /////

	///// ----- メンバ変数 ----- /////

	// このグループの設定
	ParticleGroupSettings settings_;

	// 描画を担当するInstancingParticle
	InstancingParticle instancingParticle_;

	// 描画に使うBlendMode (光るパーティクルなので加算)
	BlendMode blendMode_ = BlendMode::kAdd;

	// 乱数生成器
	std::mt19937 randomEngine_;

	///// ----- ===== ----- /////
};

///// ----- ===== ----- /////
