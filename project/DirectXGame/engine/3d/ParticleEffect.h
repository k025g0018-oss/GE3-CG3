#pragma once

#include "InstancingParticle.h"

#include <d3d12.h>
#include <memory>
#include <vector>

// 前方宣言
class TextureManager;

///// ----- パーティクルエフェクト ----- /////

/// <summary>
/// 形状ごとのInstancingParticle (板・三角錐・立方体など) を組み合わせて、1つの演出として管理する
/// ->できたらいいな
/// 形状ごとに最大数・大きさ・動きを個別に設定する
/// </summary>
class ParticleEffect {
public:
	///// ----- メンバ関数 ----- /////

	ParticleEffect() = default;
	~ParticleEffect() = default;

	ParticleEffect(const ParticleEffect&) = delete;
	ParticleEffect& operator=(const ParticleEffect&) = delete;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="device"></param>
	void Initialize(ID3D12Device* device);

	/// <summary>
	/// 更新
	/// </summary>
	/// <param name="viewMatrix"></param>
	/// <param name="projectionMatrix"></param>
	void Update(const Matrix4x4& viewMatrix, const Matrix4x4& projectionMatrix);

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="commandList"></param>
	/// <param name="textureManager"></param>
	void Draw(ID3D12GraphicsCommandList* commandList, const TextureManager& textureManager) const;

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

	///// ----- ===== ----- /////

	///// ----- getter ----- /////

	///// ----- ===== ----- /////

	///// ----- setter ----- /////

	///// ----- ===== ----- /////

private:
	///// ----- メンバ関数 ----- /////

	///// ----- ===== ----- /////

	///// ----- メンバ変数 ----- /////

	// 形状ごとのパーティクル (コピー禁止のクラスなのでunique_ptrで持つ)
	std::vector<std::unique_ptr<InstancingParticle>> groups_;

	///// ----- ===== ----- /////
};

