#pragma once

#include "BlendState.h"
#include "DepthStencilState.h"
#include "InputLayout.h"
#include "RasterizerState.h"
#include "RootSignature.h"
#include "ShaderProgram.h"

#include <array>
#include <d3d12.h>
#include <wrl.h>
#include <ostream>
#include <string>

// 前方宣言
class DxcCompiler;

///// ----- PSO(Pipeline State Object) ----- /////

class PipelineState {
public:
	PipelineState() = default;
	~PipelineState();

	PipelineState(const PipelineState&) = delete;
	PipelineState& operator=(const PipelineState&) = delete;

	/// <summary>
	/// 初期化
	/// 全てのBlendMode分のPSOを作成
	/// </summary>
	/// <param name="device"></param>
	/// <param name="dxcCompiler"></param>
	/// <param name="logStream"></param>
	/// <param name="vertexShaderPath">省略時はObject3dのVertexShader</param>
	/// <param name="pixelShaderPath">省略時はObject3dのPixelShader</param>
	/// <param name="isInstancing">trueならParticle用のRootSignatureを使う</param>
	void Initialize(
		ID3D12Device* device,
		DxcCompiler* dxcCompiler,
		std::ostream& logStream,
		const std::wstring& vertexShaderPath = L"resources/shaders/Object3d.VS.hlsl",
		const std::wstring& pixelShaderPath = L"resources/shaders/Object3d.PS.hlsl",
		bool isInstancing = false);

	/// --- 取得 ---
	// 作成したRootSignatureを取得
	ID3D12RootSignature* GetRootSignature() const { return rootSignature_.Get(); }

	// 指定したBlendModeのPSOを取得
	ID3D12PipelineState* GetPipelineState(BlendMode blendMode) const {
		return graphicsPipelineStates_[static_cast<size_t>(blendMode)].Get();
	}

	/// --- 終了処理 ---
	// PSOで使用したリソースを解放
	void Finalize();

private:
	RootSignature rootSignature_;
	InputLayout inputLayout_;
	RasterizerState rasterizerState_;
	DepthStencilState depthStencilState_;
	ShaderProgram shaderProgram_;
	// BlendModeごとのPSO (寿命をComPtrで管理する)
	std::array<Microsoft::WRL::ComPtr<ID3D12PipelineState>, static_cast<size_t>(BlendMode::kCountOf)> graphicsPipelineStates_;
};
