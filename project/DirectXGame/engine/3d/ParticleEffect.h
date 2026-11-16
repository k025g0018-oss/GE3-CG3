#pragma once

#include "ParticleGroup.h"

#include <cstdint>
#include <d3d12.h>
#include <memory>
#include <vector>

// 前方宣言
class TextureManager;
class PipelineState;
class TuningFile;

///// ----- パーティクルエフェクト ----- /////

/// <summary>
/// 種類ごとのParticleGroup (ぽわぽわ・星・三角錐・立方体など) を組み合わせて、1つの演出として管理する
/// 種類ごとに最大数・大きさ・動きを個別に設定する
/// </summary>
class ParticleEffect {
public:
	///// ----- メンバ関数 ----- /////

	// 作れるグループの最大数 (Instancing用SRVをこの数だけ予約する)
	static constexpr uint32_t kMaxGroupCount = 8;

	ParticleEffect() = default;
	~ParticleEffect() = default;

	ParticleEffect(const ParticleEffect&) = delete;
	ParticleEffect& operator=(const ParticleEffect&) = delete;

	/// <summary>
	/// 初期化
	/// 種類ごとの設定を作り、ParticleGroupを追加する
	/// </summary>
	/// <param name="device"></param>
	/// <param name="srvDescriptorHeap"></param>
	/// <param name="srvStartIndex">Instancing用SRVを作り始める番号 (グループの数だけ使う)</param>
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
	void Update(const Matrix4x4& viewMatrix, const Matrix4x4& projectionMatrix, float deltaTime);

	/// <summary>
	/// 描画
	/// グループごとにBlendModeとテクスチャを切り替えて描画する
	/// Particle用のRootSignatureを設定してから呼ぶ
	/// </summary>
	/// <param name="commandList"></param>
	/// <param name="textureManager"></param>
	/// <param name="particlePipelineState">Particle用のPSO (Depthを書き込まない)</param>
	/// <param name="solidParticlePipelineState">不透明な立体用のPSO (Depthを書き込む)</param>
	void Draw(
		ID3D12GraphicsCommandList* commandList,
		const TextureManager& textureManager,
		const PipelineState& particlePipelineState,
		const PipelineState& solidParticlePipelineState) const;

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// 全てのグループの調整値を、保存・読み込みできるように登録する (effect.グループ名.項目名)
	/// </summary>
	/// <param name="tuningFile"></param>
	void RegisterTuning(TuningFile& tuningFile);

	///// ----- ===== ----- /////

	///// ----- getter ----- /////

	/// <summary>
	/// Propertiesからグループごとの数や色を操作する
	/// </summary>
	/// <returns></returns>
	std::vector<std::unique_ptr<ParticleGroup>>& GetGroups() {
		return groups_;
	}

	///// ----- ===== ----- /////

private:
	///// ----- メンバ関数 ----- /////

	/// <summary>
	/// 設定を受け取って、ParticleGroupを1つ追加する
	/// </summary>
	/// <param name="settings"></param>
	void AddGroup(const ParticleGroupSettings& settings);

	///// ----- ===== ----- /////

	///// ----- メンバ変数 ----- /////

	// 種類ごとのパーティクル (コピー禁止のクラスなのでunique_ptrで持つ)
	std::vector<std::unique_ptr<ParticleGroup>> groups_;

	// グループを作るときに使う
	ID3D12Device* device_ = nullptr;
	const DescriptorHeap* srvDescriptorHeap_ = nullptr;
	// 次のグループが使うInstancing用SRVの番号
	uint32_t nextSrvIndex_ = 0;

	///// ----- ===== ----- /////
};

///// ----- ===== ----- /////
