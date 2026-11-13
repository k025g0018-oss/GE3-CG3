#include "InstancingParticle.h"
#include "BufferResource.h"

#include <cassert>
#include <numbers>

/// <summary>
/// 解放処理
/// </summary>
InstancingParticle::~InstancingParticle() {
	Finalize();
}

/// <summary>
/// 初期化
/// </summary>
/// <param name="device"></param>
/// <param name="srvDescriptorHeap"></param>
/// <param name="srvIndex"></param>
/// <param name="vertices"></param>
/// <param name="maxInstanceCount"></param>
void InstancingParticle::Initialize(
	ID3D12Device* device,
	const DescriptorHeap& srvDescriptorHeap,
	uint32_t srvIndex,
	const std::vector<VertexData>& vertices,
	uint32_t maxInstanceCount) {

	assert(device != nullptr);
	assert(!vertices.empty());
	assert(maxInstanceCount > 0);

	/// --- 数 ---

	// 最大数描画 最初
	maxInstanceCount_ = maxInstanceCount;
	instanceCount_ = maxInstanceCount;

	/// --- 頂点 ---

	// 受け取った形状を頂点バッファへ書き込む
	vertices_ = vertices;
	vertexBuffer_.Initialize(device, static_cast<uint32_t>(vertices_.size()));

	VertexData* vertexData = vertexBuffer_.GetData();
	for (size_t i = 0; i < vertices_.size(); ++i) {
		vertexData[i] = vertices_[i];
	}

	/// --- マテリアル ---

	materialResource_ = BufferResource::Create(device, sizeof(Material));
	HRESULT hr = materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	assert(SUCCEEDED(hr));

	// 白色、ライティングなし、uv変化なし
	materialData_->color = {1.0f, 1.0f, 1.0f, 1.0f};
	materialData_->enableLighting = false;
	materialData_->lightingMode = 0;
	materialData_->uvTransform = Matrix4x4::MakeIdentity4x4();

	/// --- Instancing用のResource ---

	// 粒の最大数分のParticleForGPUを格納できるResourceを作る
	instancingResource_ = BufferResource::Create(device, sizeof(ParticleForGPU) * maxInstanceCount_);
	// 書き込むためのアドレスを取得
	hr = instancingResource_->Map(0, nullptr, reinterpret_cast<void**>(&instancingData_));
	assert(SUCCEEDED(hr));

	// 単位行列と白色を書き込んでおく
	for (uint32_t index = 0; index < maxInstanceCount_; ++index) {
		instancingData_[index].WVP = Matrix4x4::MakeIdentity4x4();
		instancingData_[index].World = Matrix4x4::MakeIdentity4x4();
		instancingData_[index].color = {1.0f, 1.0f, 1.0f, 1.0f};
	}

	/// --- Instancing用のSRV ---

	// StructuredBufferとしてシェーダーから読めるようにする
	D3D12_SHADER_RESOURCE_VIEW_DESC instancingSrvDesc{};
	instancingSrvDesc.Format = DXGI_FORMAT_UNKNOWN; // 構造体なのでフォーマットは不明
	instancingSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	instancingSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER; // Textureではなく、Bufferとして利用する
	instancingSrvDesc.Buffer.FirstElement = 0;
	instancingSrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	instancingSrvDesc.Buffer.NumElements = maxInstanceCount_; // アクセスする要素数
	instancingSrvDesc.Buffer.StructureByteStride = sizeof(ParticleForGPU); // 1要素のサイズ (Strideが変わったのでParticleForGPUにする)

	// ヒープの指定された番号にInstancing用のSRVを作成する
	const D3D12_CPU_DESCRIPTOR_HANDLE instancingSrvHandleCPU = srvDescriptorHeap.GetCPUHandle(srvIndex);
	instancingSrvHandleGPU_ = srvDescriptorHeap.GetGPUHandle(srvIndex);
	device->CreateShaderResourceView(instancingResource_.Get(), &instancingSrvDesc, instancingSrvHandleCPU);

	/// --- 粒ごとのTransformと速度 ---

	// 乱数生成器をランダムなシードで初期化する
	std::random_device seedGenerator;
	randomEngine_.seed(seedGenerator());

	// 最初の並びに設定する
	particles_.resize(maxInstanceCount_);
	ResetParticles();

	// 片面の板の表側が初期状態でカメラに向くようにする
	transform_.rotate.y = std::numbers::pi_v<float>;
}

/// <summary>
/// 更新
/// </summary>
/// <param name="viewMatrix"></param>
/// <param name="projectionMatrix"></param>
/// <param name="deltaTime"></param>
void InstancingParticle::Update(const Matrix4x4& viewMatrix, const Matrix4x4& projectionMatrix, float deltaTime) {

	// 描画数が確保したResourceの数を超えないようにする
	assert(instanceCount_ <= maxInstanceCount_);

	/// --- 移動 ---

	// 動かす設定のときだけ、速度(m/s) × 経過時間(s) で位置を進める
	if (updateMode_ != ParticleUpdateMode::kNone) {
		for (uint32_t index = 0; index < instanceCount_; ++index) {
			particles_[index].transform.translate =
				particles_[index].transform.translate + particles_[index].velocity * deltaTime;
		}
	}

	/// --- 共通の行列 ---

	// 親のTransformからWorld行列を作る (全体をまとめて動かす)
	const Matrix4x4 parentWorldMatrix = Matrix4x4::MakeAffineMatrix(
		transform_.scale,
		transform_.rotate,
		transform_.translate
	);

	// View, Projectionは全粒共通なので先にかけておく
	const Matrix4x4 viewProjectionMatrix = Matrix4x4::Multiply(viewMatrix, projectionMatrix);

	/// --- 粒ごとの行列 ---

	for (uint32_t index = 0; index < instanceCount_; ++index) {
		// 粒自身のTransformから行列を作る
		const Matrix4x4 localMatrix = Matrix4x4::MakeAffineMatrix(
			particles_[index].transform.scale,
			particles_[index].transform.rotate,
			particles_[index].transform.translate
		);

		// 粒 -> 親 の順にかけてWorld行列を作る
		const Matrix4x4 worldMatrix = Matrix4x4::Multiply(localMatrix, parentWorldMatrix);

		// World, View, Projection をかけてWVP行列を作り、StructuredBufferへ書き込む
		instancingData_[index].WVP = Matrix4x4::Multiply(worldMatrix, viewProjectionMatrix);
		instancingData_[index].World = worldMatrix;

		// 色はそのままコピーする
		instancingData_[index].color = particles_[index].color;
	}
}

/// <summary>
/// 描画
/// </summary>
/// <param name="commandList"></param>
/// <param name="texterHandle"></param>
void InstancingParticle::Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE texterHandle) const {

	assert(commandList != nullptr);

	// 描画数が0なら何もしない
	if (instanceCount_ == 0) {
		return;
	}

	// 確保したResourceの数を超えないようにする
	assert(instanceCount_ <= maxInstanceCount_);

	// 形状の頂点バッファを設定する
	const D3D12_VERTEX_BUFFER_VIEW& vertexBufferView = vertexBuffer_.GetView();
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// マテリアルを設定する
	commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	// Instancing用のDataを読むために、StructuredBufferのSRVを設定する
	commandList->SetGraphicsRootDescriptorTable(1, instancingSrvHandleGPU_);
	// テクスチャを設定する
	commandList->SetGraphicsRootDescriptorTable(2, texterHandle);

	// 板ポリゴンを、描画数だけInstance描画する
	commandList->DrawInstanced(static_cast<UINT>(vertices_.size()), instanceCount_, 0, 0);
}

/// <summary>
/// 終了
/// </summary>
void InstancingParticle::Finalize() {

	vertexBuffer_.Finalize();
	materialData_ = nullptr;
	instancingData_ = nullptr;
}

/// <summary>
/// 動かし方を切り替える
/// </summary>
/// <param name="updateMode"></param>
void InstancingParticle::SetUpdateMode(ParticleUpdateMode updateMode) {

	updateMode_ = updateMode;

	if (updateMode_ == ParticleUpdateMode::kRandom) {
		// 生成関数を使って、全ての粒をランダムに作り直す
		for (uint32_t index = 0; index < maxInstanceCount_; ++index) {
			particles_[index] = MakeNewParticle(randomEngine_);
		}
	} else {
		// それ以外は最初の並びに戻す
		ResetParticles();
	}
}

/// <summary>
/// 粒を最初の並びに戻す
/// </summary>
void InstancingParticle::ResetParticles() {

	for (uint32_t index = 0; index < maxInstanceCount_; ++index) {
		// それぞれ位置が少しずつずれるように並べる
		particles_[index].transform.scale = {1.0f, 1.0f, 1.0f};
		particles_[index].transform.rotate = {0.0f, 0.0f, 0.0f};
		particles_[index].transform.translate = {index * 0.1f, index * 0.1f, index * 0.1f};

		// 速度を上向きに設定 (1m/s)
		particles_[index].velocity = {0.0f, 1.0f, 0.0f};

		// 色は白にする
		particles_[index].color = {1.0f, 1.0f, 1.0f, 1.0f};
	}
}

/// <summary>
/// 位置と速度がランダムな粒を1つ生成する
/// </summary>
/// <param name="randomEngine"></param>
/// <returns></returns>
Particle InstancingParticle::MakeNewParticle(std::mt19937& randomEngine) {

	// -1.0～1.0の範囲に収まる一様な乱数を作る分布生成器
	std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);

	Particle particle;
	particle.transform.scale = {1.0f, 1.0f, 1.0f};
	particle.transform.rotate = {0.0f, 0.0f, 0.0f};

	// 位置と速度を[-1,1]でランダムに初期化 (位置はm、速度はm/s)
	particle.transform.translate = {
		distribution(randomEngine),
		distribution(randomEngine),
		distribution(randomEngine)
	};
	particle.velocity = {
		distribution(randomEngine),
		distribution(randomEngine),
		distribution(randomEngine)
	};

	// 0.0～1.0の範囲でRGBをランダムに決める (αは1.0)
	std::uniform_real_distribution<float> distColor(0.0f, 1.0f);
	particle.color = {
		distColor(randomEngine),
		distColor(randomEngine),
		distColor(randomEngine),
		1.0f
	};

	return particle;
}
