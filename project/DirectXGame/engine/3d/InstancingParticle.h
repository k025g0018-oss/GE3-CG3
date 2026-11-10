#pragma once

#include "Material.h"
#include "Matrix4x4.h"
#include "TransformationMatrix.h"
#include "VertexBuffer.h"
#include "VertexData.h"

#include <cstdint>
#include <d3d12.h>
#include <vector>
#include <wrl.h>

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
	/// <param name="vertices">PrimitiveMeshで作った形状</param>
	/// <param name="maxInstanceCount">このパーティクルの最大数</param>
	void Initialize(ID3D12Device* device, const std::vector<VertexData>& vertices, uint32_t maxInstanceCount);

	/// <summary>
	/// 更新
	/// Transformからwvp行列を作る
	/// </summary>
	/// <param name="viewMatrix"></param>
	/// <param name="projectionMatrix"></param>
	void Update(const Matrix4x4& viewMatrix, const Matrix4x4& projectionMatrix);

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

	///// ----- ===== ----- /////

private:
	///// ----- メンバ関数 ----- /////

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

	// インスタンスで1つのwvpを共有する
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;
	TransformationMatrix* wvpData_ = nullptr;

	Transform transform_{
		{1.0f, 1.0f, 1.0f},
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f}
	};

	///// ----- ===== ----- /////

	///// -----  ----- /////

	///// ----- ===== ----- /////

};

