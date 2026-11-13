#include "ParticleEffect.h"
#include "PipelineState.h"
#include "PrimitiveMesh.h"
#include "TextureManager.h"

#include <cassert>
#include <numbers>

/// <summary>
/// 初期化
/// </summary>
/// <param name="device"></param>
/// <param name="srvDescriptorHeap"></param>
/// <param name="srvStartIndex"></param>
/// <param name="textureManager"></param>
void ParticleEffect::Initialize(
	ID3D12Device* device,
	const DescriptorHeap& srvDescriptorHeap,
	uint32_t srvStartIndex,
	const TextureManager& textureManager) {

	device_ = device;
	srvDescriptorHeap_ = &srvDescriptorHeap;
	nextSrvIndex_ = srvStartIndex;

	/// --- 画面の範囲 ---

	// カメラ(Z=-10、fovY=0.45)から見て、Z=0付近で画面全体に収まる範囲 (m)
	const Vector3 screenMin = {-4.0f, -2.3f, -1.0f};
	const Vector3 screenMax = {4.0f, 2.3f, 1.0f};

	/// --- ぽわぽわ ---

	// ランダムな位置に発生し、ランダムな方向へ漂って消える (大きさは変化しない)
	ParticleGroupSettings powapowa;
	powapowa.name = "Powapowa";
	powapowa.vertices = PrimitiveMesh::CreateQuad();
	powapowa.maxCount = 50;
	powapowa.textureIndex = textureManager.FindTextureIndex("powapowaParticle.png");
	powapowa.spawnMin = screenMin;
	powapowa.spawnMax = screenMax;
	powapowa.scaleMin = 0.2f;
	powapowa.scaleMax = 0.6f;
	powapowa.speedMin = 0.2f;
	powapowa.speedMax = 0.8f;
	powapowa.lifeTimeMin = 2.0f;
	powapowa.lifeTimeMax = 5.0f;
	powapowa.fadeInRatio = 0.2f;
	AddGroup(powapowa);

	/// --- 星 ---

	// ランダムな位置に発生し、その場で回転しながら大きくなったり小さくなったりして消える
	ParticleGroupSettings star;
	star.name = "Star";
	star.vertices = PrimitiveMesh::CreateQuad();
	star.maxCount = 50;
	star.textureIndex = textureManager.FindTextureIndex("starParticle.png");
	star.spawnMin = screenMin;
	star.spawnMax = screenMax;
	star.scaleMin = 0.2f;
	star.scaleMax = 0.5f;
	star.angularSpeedMin = 0.5f * std::numbers::pi_v<float>;
	star.angularSpeedMax = 2.0f * std::numbers::pi_v<float>;
	star.scalePulseAmount = 0.4f;
	star.scalePulseSpeedMin = 2.0f;
	star.scalePulseSpeedMax = 6.0f;
	star.lifeTimeMin = 1.0f;
	star.lifeTimeMax = 3.0f;
	star.fadeInRatio = 0.2f;
	AddGroup(star);

	/// --- 三角錐・立方体 ---

	// どちらも同じ大きさで、ランダムな位置にずっと残り、ランダムな向きに回転し続ける
	// 色は粒ごとにランダム、陰影をつけて立体に見せる
	ParticleGroupSettings pyramid;
	pyramid.name = "Pyramid";
	pyramid.vertices = PrimitiveMesh::CreatePyramid();
	pyramid.maxCount = 10;
	pyramid.textureIndex = textureManager.FindTextureIndex("white.png");
	pyramid.spawnMin = screenMin;
	pyramid.spawnMax = screenMax;
	pyramid.scaleMin = 0.2f;
	pyramid.scaleMax = 0.2f;
	pyramid.angularSpeedMin = 0.5f * std::numbers::pi_v<float>;
	pyramid.angularSpeedMax = 1.0f * std::numbers::pi_v<float>;
	pyramid.isRotateAllAxes = true;
	pyramid.isImmortal = true;
	pyramid.isRandomColor = true;
	pyramid.isLightingEnabled = true;
	pyramid.isSolid = true;
	AddGroup(pyramid);

	// 立方体は形状と名前だけ変える
	ParticleGroupSettings cube = pyramid;
	cube.name = "Cube";
	cube.vertices = PrimitiveMesh::CreateCube();
	AddGroup(cube);
}

/// <summary>
/// 更新
/// </summary>
/// <param name="viewMatrix"></param>
/// <param name="projectionMatrix"></param>
/// <param name="deltaTime"></param>
void ParticleEffect::Update(const Matrix4x4& viewMatrix, const Matrix4x4& projectionMatrix, float deltaTime) {

	for (std::unique_ptr<ParticleGroup>& group : groups_) {
		group->Update(viewMatrix, projectionMatrix, deltaTime);
	}
}

/// <summary>
/// 描画
/// </summary>
/// <param name="commandList"></param>
/// <param name="textureManager"></param>
/// <param name="particlePipelineState"></param>
/// <param name="solidParticlePipelineState"></param>
void ParticleEffect::Draw(
	ID3D12GraphicsCommandList* commandList,
	const TextureManager& textureManager,
	const PipelineState& particlePipelineState,
	const PipelineState& solidParticlePipelineState) const {

	/// --- 不透明な立体 ---

	// 先にDepthを書き込んでおき、後から描く光る粒が立体の奥にあるときは隠れるようにする
	for (const std::unique_ptr<ParticleGroup>& group : groups_) {
		if (!group->IsSolid()) {
			continue;
		}

		// グループのBlendModeのPSO (Depthを書き込む) に切り替える
		commandList->SetPipelineState(solidParticlePipelineState.GetPipelineState(group->GetBlendMode()));

		// グループのテクスチャを貼って描画する
		group->Draw(commandList, textureManager.GetSrvHandle(group->GetTextureIndex()));
	}

	/// --- 光る粒 ---

	for (const std::unique_ptr<ParticleGroup>& group : groups_) {
		if (group->IsSolid()) {
			continue;
		}

		// グループのBlendModeのPSO (Depthを書き込まない) に切り替える
		commandList->SetPipelineState(particlePipelineState.GetPipelineState(group->GetBlendMode()));

		// グループのテクスチャを貼って描画する
		group->Draw(commandList, textureManager.GetSrvHandle(group->GetTextureIndex()));
	}
}

/// <summary>
/// 解放処理
/// </summary>
void ParticleEffect::Finalize() {

	for (std::unique_ptr<ParticleGroup>& group : groups_) {
		group->Finalize();
	}
	groups_.clear();
}

/// <summary>
/// 設定を受け取って、ParticleGroupを1つ追加する
/// </summary>
/// <param name="settings"></param>
void ParticleEffect::AddGroup(const ParticleGroupSettings& settings) {

	// 予約したSRVの数を超えないようにする
	assert(groups_.size() < kMaxGroupCount);

	std::unique_ptr<ParticleGroup> group = std::make_unique<ParticleGroup>();

	// グループごとに別のInstancing用SRVを使う
	group->Initialize(device_, *srvDescriptorHeap_, nextSrvIndex_, settings);
	++nextSrvIndex_;

	groups_.push_back(std::move(group));
}
