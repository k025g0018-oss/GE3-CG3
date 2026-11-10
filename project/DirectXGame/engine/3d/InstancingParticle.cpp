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
/// <param name="vertices"></param>
/// <param name="maxInstanceCount"></param>
void InstancingParticle::Initialize(ID3D12Device* device, const std::vector<VertexData>& vertices, uint32_t maxInstanceCount) {

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

	/// --- WVP ---

	wvpResource_ = BufferResource::Create(device, sizeof(TransformationMatrix));
	hr = wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&wvpData_));
	assert(SUCCEEDED(hr));

	wvpData_->WVP = Matrix4x4::MakeIdentity4x4();
	wvpData_->World = Matrix4x4::MakeIdentity4x4();

	// 片面の板の表側が初期状態でカメラに向くようにする
	transform_.rotate.y = std::numbers::pi_v<float>;
}

/// <summary>
/// 更新
/// </summary>
/// <param name="viewMatrix"></param>
/// <param name="projectionMatrix"></param>
void InstancingParticle::Update(const Matrix4x4& viewMatrix, const Matrix4x4& projectionMatrix) {

	// TransformからWorld行列を作る
	const Matrix4x4 worldMatrix = Matrix4x4::MakeAffineMatrix(
		transform_.scale,
		transform_.rotate,
		transform_.translate
	);

	// World, View, Projection をかけてWVP行列を作る
	wvpData_->WVP = Matrix4x4::Multiply(worldMatrix, Matrix4x4::Multiply(viewMatrix, projectionMatrix));
	wvpData_->World = worldMatrix;
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

	// 形状の頂点バッファを設定する
	const D3D12_VERTEX_BUFFER_VIEW& vertexBufferView = vertexBuffer_.GetView();
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// マテリアル・WVP・テクスチャを設定する
	commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(2, texterHandle);

	// 第2引数が描画する数 -> 今は全部同じwvpなので同じ位置に重なる
	commandList->DrawInstanced(static_cast<UINT>(vertices_.size()), instanceCount_, 0, 0);
}

/// <summary>
/// 終了
/// </summary>
void InstancingParticle::Finalize() {

	vertexBuffer_.Finalize();
	materialData_ = nullptr;
	wvpData_ = nullptr;
}