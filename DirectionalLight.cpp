#include "DirectionalLight.h"
#include "BufferResource.h"

#include <cassert>
#include <cmath>

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif // USE_IMGUI

///// ----- 初期化 ----- /////

void DirectionalLight::Initialize(ID3D12Device* device) {

	/// --- 定数バッファ ---

	// 平行光源用の定数バッファを作成する
	resource_ = BufferResource::Create(device, sizeof(Data));

	// 書き込むためのアドレスを取得
	HRESULT hr = resource_->Map(0, nullptr, reinterpret_cast<void**>(&data_));
	assert(SUCCEEDED(hr));

	/// --- デフォルト値 ---

	// 平行光源の色を白に設定する
	data_->color = {1.0f, 1.0f, 1.0f, 1.0f};
	// 真下へ進む単位ベクトルを設定する
	data_->direction = {0.0f, -1.0f, 0.0f};
	// 平行光源の明るさを設定する
	data_->intensity = 1.0f;
}

///// ----- ImGui ----- /////

void DirectionalLight::DrawImGui() {
#ifdef USE_IMGUI

	// 光源色を変更する
	ImGui::ColorEdit4("Light Color", &data_->color.x);

	// 光の進む方向を変更する
	const bool directionChanged = ImGui::DragFloat3(
		"Light Direction",
		&data_->direction.x,
		0.01f,
		-1.0f,
		1.0f);

	// 光源の輝度を変更する
	ImGui::DragFloat(
		"Light Intensity",
		&data_->intensity,
		0.01f,
		0.0f,
		10.0f);

	// 方向が変更されたら必ず単位ベクトルへ正規化する
	if (directionChanged) {
		NormalizeDirection();
	}

#endif // USE_IMGUI
}

///// ----- GPUへのセット ----- /////

void DirectionalLight::Bind(ID3D12GraphicsCommandList* commandList, UINT rootParameterIndex) const {
	// 平行光源の定数バッファをPixel Shaderへ設定する
	commandList->SetGraphicsRootConstantBufferView(rootParameterIndex, resource_->GetGPUVirtualAddress());
}

///// ----- 正規化 ----- /////

void DirectionalLight::NormalizeDirection() {
	Vector3& direction = data_->direction;

	const float length = std::sqrt(
		direction.x * direction.x +
		direction.y * direction.y +
		direction.z * direction.z);

	// ゼロベクトルは正規化できないため除外する
	if (length > 0.0001f) {
		direction.x /= length;
		direction.y /= length;
		direction.z /= length;
	} else {
		// 不正な方向になった場合は真下へ戻す
		direction = {0.0f, -1.0f, 0.0f};
	}
}