#include "ParticleEffect.h"

#include "ParticleEffect.h"
#include "TextureManager.h"

/// <summary>
/// 初期化
/// </summary>
/// <param name="device"></param>
void ParticleEffect::Initialize(ID3D12Device* device) {

	// 形状ごとのInstancingParticleを追加する
	(void)device;
}

/// <summary>
/// 更新
/// </summary>
/// <param name="viewMatrix"></param>
/// <param name="projectionMatrix"></param>
void ParticleEffect::Update(const Matrix4x4& viewMatrix, const Matrix4x4& projectionMatrix) {

	for (std::unique_ptr<InstancingParticle>& group : groups_) {
		group->Update(viewMatrix, projectionMatrix);
	}
}

/// <summary>
/// 描画
/// </summary>
/// <param name="commandList"></param>
/// <param name="textureManager"></param>
void ParticleEffect::Draw(ID3D12GraphicsCommandList* commandList, const TextureManager& textureManager) const {

	// 形状ごとのテクスチャを決めて描画する
	(void)commandList;
	(void)textureManager;
}

/// <summary>
/// 解放処理
/// </summary>
void ParticleEffect::Finalize() {

	for (std::unique_ptr<InstancingParticle>& group : groups_) {
		group->Finalize();
	}
	groups_.clear();
}