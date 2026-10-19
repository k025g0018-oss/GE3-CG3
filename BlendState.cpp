#include "BlendState.h"

/// <summary>
/// 初期化
/// 指定したBlendModeで色の書き込み方法を設定
/// </summary>
/// <param name="blendMode"></param>
void BlendState::Initialize(BlendMode blendMode) {

	// 全ての色要素を書き込む
	blendDesc_.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	switch (blendMode) {
		// 合成しない (これから描く色がそのまま書き込まれる)
		case BlendMode::kNone:
			blendDesc_.RenderTarget[0].BlendEnable = FALSE;
			break;

			// NormalBlend : Result = SrcColor * SrcAlpha + DestColor * (1 - SrcAlpha)
		case BlendMode::kNormal:
			blendDesc_.RenderTarget[0].BlendEnable = TRUE;
			blendDesc_.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
			blendDesc_.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
			blendDesc_.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
			// α値のブレンド設定 (基本的に使わないので、この値のままにしておく)
			blendDesc_.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
			blendDesc_.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
			blendDesc_.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
			break;
	}
}
