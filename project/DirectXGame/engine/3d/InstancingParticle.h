#pragma once

#include "BlendState.h"
#include "DescriptorHeap.h"
#include "Material.h"
#include "Matrix4x4.h"
#include "TransformationMatrix.h"
#include "VertexBuffer.h"
#include "VertexData.h"

#include <cstdint>
#include <d3d12.h>
#include <limits>
#include <random>
#include <vector>
#include <wrl.h>

///// ----- パーティクル1粒 ----- /////

/// <summary>
/// 移動させるためにTransformと速度を持つ
/// </summary>
struct Particle {
	Transform transform;
	// 速度 (m/s)
	Vector3 velocity;
	// 色 (RGBA)
	Vector4 color;
	// 生存可能な時間 (s)
	float lifeTime;
	// 発生してからの経過時間 (s)
	float currentTime;
	// 回転の速さ (rad/s)
	Vector3 angularVelocity{};
	// 発生時の大きさ (拡縮の基準)
	Vector3 baseScale{1.0f, 1.0f, 1.0f};
	// 拡縮の速さ (rad/s) 0なら大きさは変化しない
	float scalePulseSpeed = 0.0f;
	// 色相 (0.0～1.0) 色をだんだん変えるときに使う
	float hue = 0.0f;
};

///// ----- ===== ----- /////

///// ----- GPUに転送するパーティクルのデータ ----- /////

/// <summary>
/// GPUが描画に利用するデータ (シェーダーのParticleForGPUと同じ並びにする)
/// Particle(CPUで動かす挙動のデータ)とは別物
/// </summary>
struct ParticleForGPU {
	Matrix4x4 WVP;
	Matrix4x4 World;
	Vector4 color;
	// Dissolve (αTest) の閾値 テクスチャのα値がこれ以下のPixelは描画しない
	float dissolveThreshold;
};

///// ----- ===== ----- /////

///// ----- 更新の種類 ----- /////

/// <summary>
/// パーティクルの動かし方
/// </summary>
enum class ParticleUpdateMode {
	kNone,   // 動かさない (初期の並び)
	kNormal, // 真上に動く
	kRandom, // ランダムな位置・速度で動く
};

///// ----- ===== ----- /////

///// ----- インスタンシングParticle ----- /////

/// <summary>
/// 1つの形状を、DrawInstancedで最大数までまとめて描画する
/// </summary>
class InstancingParticle {
public:
	///// ----- メンバ関数 ----- /////

	InstancingParticle() = default;
	~InstancingParticle();

	// DirectX12リソースの二重解放を防ぐため、コピー禁止
	InstancingParticle(const InstancingParticle&) = delete;
	InstancingParticle& operator= (const InstancingParticle&) = delete;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="device"></param>
	/// <param name="srvDescriptorHeap">Instancing用のSRVを作るヒープ</param>
	/// <param name="srvIndex">ヒープの空いている番号</param>
	/// <param name="vertices">PrimitiveMeshで作った形状</param>
	/// <param name="maxInstanceCount">このパーティクルの最大数</param>
	void Initialize(
		ID3D12Device* device,
		const DescriptorHeap& srvDescriptorHeap,
		uint32_t srvIndex,
		const std::vector<VertexData>& vertices,
		uint32_t maxInstanceCount);

	/// <summary>
	/// 更新
	/// 速度で粒を動かし (SimulateParticles)、StructuredBufferへ書き込む (TransferToGPU)
	/// </summary>
	/// <param name="viewMatrix"></param>
	/// <param name="projectionMatrix"></param>
	/// <param name="deltaTime">前のフレームからの経過時間 (秒)</param>
	void Update(const Matrix4x4& viewMatrix, const Matrix4x4& projectionMatrix, float deltaTime);

	/// <summary>
	/// GPUへの転送
	/// 生きている粒だけを前から詰めて、wvp・World・色などをStructuredBufferへ書き込む
	/// 動かし方を外側 (ParticleGroupなど) で決めるときは、粒を動かした後にこれだけを呼ぶ
	/// </summary>
	/// <param name="viewMatrix"></param>
	/// <param name="projectionMatrix"></param>
	void TransferToGPU(const Matrix4x4& viewMatrix, const Matrix4x4& projectionMatrix);

	/// <summary>
	/// 描画
	/// 今の描画数だけまとめて描画する
	/// </summary>
	/// <param name="commandList"></param>
	/// <param name="textureHandle"></param>
	void Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle) const;

	/// <summary>
	/// 終了
	/// </summary>
	void Finalize();

	///// ----- ===== ----- /////

	///// ----- getter ----- /////

	/// <summary>
	/// Propertiesから位置などを操作する
	/// </summary>
	/// <returns></returns>
	Transform& GetTransform() {
		return transform_;
	}

	/// <summary>
	/// Propertiesから今の描画数を操作する (1～最大数)
	/// </summary>
	/// <returns></returns>
	uint32_t& GetInstanceCount() {
		return instanceCount_;
	}

	/// <summary>
	/// 最大数を取得する
	/// </summary>
	/// <returns></returns>
	uint32_t GetMaxInstanceCount() const {
		return maxInstanceCount_;
	}

	/// <summary>
	/// Propertiesから色を操作する
	/// </summary>
	/// <returns></returns>
	Vector4& GetColor() {
		return materialData_->color;
	}

	/// <summary>
	/// 今の動かし方を取得する
	/// </summary>
	/// <returns></returns>
	ParticleUpdateMode GetUpdateMode() const {
		return updateMode_;
	}

	/// <summary>
	/// 今のフレームで生きている (描画する) 粒の数を取得する
	/// </summary>
	/// <returns></returns>
	uint32_t GetAliveCount() const {
		return aliveCount_;
	}

	/// <summary>
	/// Propertiesから描画に使うBlendModeを操作する
	/// </summary>
	/// <returns></returns>
	BlendMode& GetBlendMode() {
		return blendMode_;
	}

	/// <summary>
	/// Propertiesから再発生するかどうかを操作する
	/// </summary>
	/// <returns></returns>
	bool& GetIsRespawn() {
		return isRespawn_;
	}

	/// <summary>
	/// PropertiesからDissolveの閾値を操作する (0.0～1.0)
	/// </summary>
	/// <returns></returns>
	float& GetDissolveThreshold() {
		return dissolveThreshold_;
	}

	/// <summary>
	/// Propertiesから寿命に合わせてDissolveさせるかどうかを操作する
	/// </summary>
	/// <returns></returns>
	bool& GetIsDissolveOverLife() {
		return isDissolveOverLife_;
	}

	/// <summary>
	/// 粒のデータを取得する (外側で動かし方を決めるときに使う)
	/// </summary>
	/// <returns></returns>
	std::vector<Particle>& GetParticles() {
		return particles_;
	}

	///// ----- ===== ----- /////

	///// ----- setter ----- /////

	/// <summary>
	/// 動かし方を切り替える
	/// 切り替えたら、粒の位置と速度を設定し直す
	/// </summary>
	/// <param name="updateMode"></param>
	void SetUpdateMode(ParticleUpdateMode updateMode);

	/// <summary>
	/// 寿命のうち、最初のどれだけの割合でフェードインさせるかを設定する (0.0ならフェードインしない)
	/// </summary>
	/// <param name="fadeInRatio"></param>
	void SetFadeInRatio(float fadeInRatio) {
		fadeInRatio_ = fadeInRatio;
	}

	/// <summary>
	/// 簡易ライティングで陰影をつけるかどうかを設定する (立体の形状用)
	/// </summary>
	/// <param name="isLightingEnabled"></param>
	void SetLightingEnabled(bool isLightingEnabled) {
		materialData_->enableLighting = isLightingEnabled;
	}

	///// ----- ===== ----- /////

private:
	///// ----- メンバ関数 ----- /////

	/// <summary>
	/// 粒を最初の並びに戻す (斜めに少しずつずらし、速度は真上)
	/// </summary>
	void ResetParticles();

	/// <summary>
	/// 動かし方 (Normal / Random) に合わせて、粒の位置と経過時間を進める
	/// </summary>
	/// <param name="deltaTime"></param>
	void SimulateParticles(float deltaTime);

	/// <summary>
	/// 位置と速度がランダムな粒を1つ生成する
	/// 消滅・生成を繰り返すときにも使う
	/// </summary>
	/// <param name="randomEngine">乱数生成器</param>
	/// <returns>生成した粒</returns>
	Particle MakeNewParticle(std::mt19937& randomEngine);

	///// ----- ===== ----- /////

	///// ----- メンバ変数 ----- /////

	// 形状の頂点
	std::vector<VertexData> vertices_;
	VertexBuffer vertexBuffer_;

	// 最大数と、使う粒の数 (ImGuiで変更する)
	uint32_t maxInstanceCount_ = 0;
	uint32_t instanceCount_ = 0;

	// 生きている粒の数 (描画すべきインスタンス数、毎フレーム数え直す)
	uint32_t aliveCount_ = 0;

	// 粒共通のマテリアル
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	Material* materialData_ = nullptr;

	// 粒ごとのTransformと速度 (最大数分)
	std::vector<Particle> particles_;

	// 今の動かし方
	ParticleUpdateMode updateMode_ = ParticleUpdateMode::kNone;

	// 描画に使うBlendMode (パーティクルは加算で調整することがほとんど)
	BlendMode blendMode_ = BlendMode::kAdd;

	// Randomのとき、寿命が尽きた粒を再発生させるかどうか
	bool isRespawn_ = false;

	// Dissolve (αTest) の閾値 上げるほど抜ける部分が増える
	float dissolveThreshold_ = 0.0f;

	// Randomのとき、寿命に合わせて閾値を上げていくかどうか
	bool isDissolveOverLife_ = false;

	// 寿命のうち、フェードインに使う割合 (0.0ならフェードインしない)
	float fadeInRatio_ = 0.0f;

	// 乱数生成器
	std::mt19937 randomEngine_;

	// 粒ごとのwvp・World・色を並べたStructuredBuffer
	Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource_;
	ParticleForGPU* instancingData_ = nullptr;

	// StructuredBufferをシェーダーから読むためのSRV
	D3D12_GPU_DESCRIPTOR_HANDLE instancingSrvHandleGPU_{};

	// 全体をまとめて動かす親のTransform
	Transform transform_{
		{1.0f, 1.0f, 1.0f},
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f}
	};

	///// ----- ===== ----- /////

	///// -----  ----- /////

	///// ----- ===== ----- /////

};

