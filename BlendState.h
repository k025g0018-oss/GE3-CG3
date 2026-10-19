#pragma once

#include <d3d12.h>

///// ----- BlendMode ----- /////

// 合成の種類
enum class BlendMode {
	kNone, // 合成しない (不透明)
	kNormal, // NormalBlend (αで半透明)
};

///// ----- ===== ----- /////

/// <summary>
/// BlendState
/// </summary>
class BlendState {
public:
	/// <summary>
	/// 初期化
	/// 指定したBlendModeで色の書き込み方法を設定
	/// </summary>
	/// <param name="blendMode"></param>
	void Initialize(BlendMode blendMode);

	/// <summary>
	/// 取得
	/// BlendStateの設定を取得
	/// </summary>
	/// <returns></returns>
	const D3D12_BLEND_DESC& GetDesc() const {
		return blendDesc_;
	}

private:
	D3D12_BLEND_DESC blendDesc_{};
};
