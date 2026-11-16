#include "DepthStencilState.h"

///// ----- DepthStencilState ----- /////

/// --- 初期化 ---
// DepthStencilStateの設定
void DepthStencilState::Initialize(bool isDepthWriteEnabled) {
	// Depthの機能を有効化する
	depthStencilDesc_.DepthEnable = true;
	if (isDepthWriteEnabled) {
		// 書き込みします
		depthStencilDesc_.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	} else {
		// Depthの書き込みを行わない (比較は行うので、3Dオブジェクトの奥には隠れる)
		depthStencilDesc_.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	}
	// 比較関数はLessEqual。つまり、近ければ描画される
	depthStencilDesc_.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
}
