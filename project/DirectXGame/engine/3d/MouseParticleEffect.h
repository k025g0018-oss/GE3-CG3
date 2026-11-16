#pragma once

#include "ParticleGroup.h"

#include <cstdint>
#include <d3d12.h>

// 前方宣言
class TextureManager;
class PipelineState;
class TuningFile;

///// ----- マウスのパーティクルエフェクト ----- /////

/// <summary>
/// マウスカーソルに合わせて出すパーティクルをまとめて管理する
/// ・クリックした場所から、立方体が花火のように上へ飛び散って落ちながら消える
/// ・カーソルの周りから、緑の円がふわっと出て小さくなりながら消える
/// ・カーソルの位置に、緑の輪を表示する
/// (AL2_評価課題2のヒットエフェクト・マウスパーティクル・カーソルの円を3Dにしたもの)
/// </summary>
class MouseParticleEffect {
public:
	///// ----- 定数 ----- /////

	// 使うInstancing用SRVの数 (立方体・円・カーソルの輪)
	static constexpr uint32_t kSrvCount = 3;

	///// ----- ===== ----- /////

	///// ----- メンバ関数 ----- /////

	MouseParticleEffect() = default;
	~MouseParticleEffect() = default;

	MouseParticleEffect(const MouseParticleEffect&) = delete;
	MouseParticleEffect& operator=(const MouseParticleEffect&) = delete;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="device"></param>
	/// <param name="srvDescriptorHeap"></param>
	/// <param name="srvStartIndex">Instancing用SRVを作り始める番号 (kSrvCount個使う)</param>
	/// <param name="textureManager">テクスチャの番号を調べるために使う</param>
	void Initialize(
		ID3D12Device* device,
		const DescriptorHeap& srvDescriptorHeap,
		uint32_t srvStartIndex,
		const TextureManager& textureManager);

	/// <summary>
	/// 更新
	/// </summary>
	/// <param name="viewMatrix"></param>
	/// <param name="projectionMatrix"></param>
	/// <param name="deltaTime">前のフレームからの経過時間 (秒)</param>
	/// <param name="mouseWorldPosition">マウスカーソルの位置 (ワールド座標、m)</param>
	/// <param name="isMouseOnScreen">マウスカーソルがゲーム画面の上にあるか</param>
	/// <param name="isMouseClicked">このフレームで左クリックされたか</param>
	void Update(
		const Matrix4x4& viewMatrix,
		const Matrix4x4& projectionMatrix,
		float deltaTime,
		const Vector3& mouseWorldPosition,
		bool isMouseOnScreen,
		bool isMouseClicked);

	/// <summary>
	/// 描画
	/// Particle用のRootSignatureを設定してから呼ぶ
	/// </summary>
	/// <param name="commandList"></param>
	/// <param name="textureManager"></param>
	/// <param name="particlePipelineState">Particle用のPSO (Depthを書き込まない)</param>
	void Draw(
		ID3D12GraphicsCommandList* commandList,
		const TextureManager& textureManager,
		const PipelineState& particlePipelineState) const;

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// 調整値を、保存・読み込みできるように登録する (mouse.項目名)
	/// </summary>
	/// <param name="tuningFile"></param>
	void RegisterTuning(TuningFile& tuningFile);

	/// <summary>
	/// 画面上の位置を、Z=planeZの平面上のワールド座標に変換する
	/// </summary>
	/// <param name="screenRatio">画面上の位置 (左上が(0,0)、右下が(1,1))</param>
	/// <param name="viewMatrix"></param>
	/// <param name="projectionMatrix"></param>
	/// <param name="planeZ">当てる平面のZ座標 (m)</param>
	/// <returns>平面上のワールド座標</returns>
	static Vector3 ScreenToWorld(
		const Vector2& screenRatio,
		const Matrix4x4& viewMatrix,
		const Matrix4x4& projectionMatrix,
		float planeZ = 0.0f);

	///// ----- ===== ----- /////

	///// ----- getter ----- /////

	/// <summary>
	/// クリックで飛び散る立方体
	/// </summary>
	ParticleGroup& GetBurstGroup() {
		return burstGroup_;
	}

	/// <summary>
	/// カーソルから出る円
	/// </summary>
	ParticleGroup& GetTrailGroup() {
		return trailGroup_;
	}

	/// <summary>
	/// カーソルの輪 (Propertiesから色や大きさを変える)
	/// </summary>
	InstancingParticle& GetCursor() {
		return cursor_;
	}

	/// <summary>
	/// 1回のクリックで出す立方体の数
	/// </summary>
	uint32_t& GetBurstCount() {
		return burstCount_;
	}

	/// <summary>
	/// 円を出す間隔 (秒)
	/// </summary>
	float& GetTrailEmitInterval() {
		return trailEmitInterval_;
	}

	/// <summary>
	/// カーソルの輪を表示するか
	/// </summary>
	bool& GetIsCursorVisible() {
		return isCursorVisible_;
	}

	///// ----- ===== ----- /////

private:
	///// ----- メンバ変数 ----- /////

	/// --- パーティクル ---

	// クリックした場所から花火のように飛び散る立方体
	ParticleGroup burstGroup_;

	// カーソルの周りから出る緑の円
	ParticleGroup trailGroup_;

	// カーソルの位置に表示する緑の輪 (1つだけ)
	InstancingParticle cursor_;

	/// --- 発生の設定 ---

	// 1回のクリックで出す立方体の数
	uint32_t burstCount_ = 10;

	// 円を出す間隔 (秒) AL2では5フレームに1個 → 5/60秒
	float trailEmitInterval_ = 5.0f / 60.0f;

	// 円を出すまでの時間を数える (秒)
	float trailEmitTimer_ = 0.0f;

	/// --- 表示 ---

	// カーソルの輪を表示するか
	bool isCursorVisible_ = true;

	// マウスカーソルがゲーム画面の上にあるか (外にあるときは輪を描画しない)
	bool isMouseOnScreen_ = false;

	// 輪に貼るテクスチャの番号 (白)
	uint32_t cursorTextureIndex_ = 0;

	///// ----- ===== ----- /////
};

///// ----- ===== ----- /////
