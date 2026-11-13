#pragma once

#include "DescriptorHeap.h"
#include "Material.h"
#include "Matrix4x4.h"
#include "TransformationMatrix.h"
#include "VertexBuffer.h"
#include "VertexData.h"

#include <cstdint>
#include <d3d12.h>
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
	/// 速度で粒を動かし、粒ごとのTransformからwvp行列を作ってStructuredBufferへ書き込む
	/// </summary>
	/// <param name="viewMatrix"></param>
	/// <param name="projectionMatrix"></param>
	/// <param name="deltaTime">前のフレームからの経過時間 (秒)</param>
	void Update(const Matrix4x4& viewMatrix, const Matrix4x4& projectionMatrix, float deltaTime);

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

	///// ----- ===== ----- /////

	///// ----- setter ----- /////

	/// <summary>
	/// 動かし方を切り替える
	/// 切り替えたら、粒の位置と速度を設定し直す
	/// </summary>
	/// <param name="updateMode"></param>
	void SetUpdateMode(ParticleUpdateMode updateMode);

	///// ----- ===== ----- /////

private:
	///// ----- メンバ関数 ----- /////

	/// <summary>
	/// 粒を最初の並びに戻す (斜めに少しずつずらし、速度は真上)
	/// </summary>
	void ResetParticles();

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

	// 最大数と、今描画している数
	uint32_t maxInstanceCount_ = 0;
	uint32_t instanceCount_ = 0;

	// 粒共通のマテリアル
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	Material* materialData_ = nullptr;

	// 粒ごとのTransformと速度 (最大数分)
	std::vector<Particle> particles_;

	// 今の動かし方
	ParticleUpdateMode updateMode_ = ParticleUpdateMode::kNone;

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

