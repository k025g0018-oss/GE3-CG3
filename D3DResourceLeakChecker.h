#pragma once

/// <summary>
/// DirectX12のリソースリークチェッカー
/// </summary>
class D3DResourceLeakChecker {
public:
	///// ----- メンバ関数 ----- /////

	/// <summary>
	/// デストラクタ (すべてのComPtrが解放された後にリソースリークを確認する)
	/// </summary>
	~D3DResourceLeakChecker();

	///// ----- ===== ----- /////
};