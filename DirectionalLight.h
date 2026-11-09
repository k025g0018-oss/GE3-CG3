#pragma once

#include "Vector.h"

#include <d3d12.h>
#include <wrl.h>

///// ----- 平行光源 ----- /////

/// <summary>
/// 平行光源
/// 定数バッファの作成、ImGuiでの調整、GPUへのセットをまとめる
/// </summary>
class DirectionalLight {
public:
	/// --- GPUに送るデータ ---
	// シェーダーのDirectionalLightと同じ並びにする
	struct Data {
		Vector4 color;     // ライトの色
		Vector3 direction; // ライトが進む方向(単位ベクトル)
		float intensity;   // ライトの明るさ
	};

	/// <summary>
	/// 初期化
	/// 定数バッファを作成し、初期値を書き込む
	/// </summary>
	/// <param name="device"></param>
	void Initialize(ID3D12Device* device);

	/// <summary>
	/// ImGui
	/// 光源の色・向き・明るさを調整する
	/// </summary>
	void DrawImGui();

	/// <summary>
	/// GPUへのセット
	/// 指定したルートパラメータの番号に定数バッファを設定する
	/// </summary>
	/// <param name="commandList"></param>
	/// <param name="rootParameterIndex"></param>
	void Bind(ID3D12GraphicsCommandList* commandList, UINT rootParameterIndex) const;

private:
	// 方向を単位ベクトルへ正規化する
	void NormalizeDirection();

	// 平行光源の定数バッファ (寿命をComPtrで管理する)
	Microsoft::WRL::ComPtr<ID3D12Resource> resource_;
	// CPUから光源情報を書き込むアドレス
	Data* data_ = nullptr;
};