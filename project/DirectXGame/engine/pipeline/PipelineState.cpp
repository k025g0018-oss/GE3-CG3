#include "PipelineState.h"

#include <cassert>

///// ----- PSO(Pipeline State Object) ----- /////

PipelineState::~PipelineState() {
	Finalize();
}

/// <summary>
/// 初期化
/// 全てのBlendMode分のPSOを作成
/// </summary>
/// <param name="device"></param>
/// <param name="dxcCompiler"></param>
/// <param name="logStream"></param>
/// <param name="vertexShaderPath"></param>
/// <param name="pixelShaderPath"></param>
/// <param name="isInstancing"></param>
/// <param name="isDepthWriteEnabled"></param>
void PipelineState::Initialize(
	ID3D12Device* device,
	DxcCompiler* dxcCompiler,
	std::ostream& logStream,
	const std::wstring& vertexShaderPath,
	const std::wstring& pixelShaderPath,
	bool isInstancing,
	bool isDepthWriteEnabled) {

	/// --- RootSignature ---

	// RootSignatureを作成 (Particle用はVertexShaderでStructuredBufferを受け取る)
	rootSignature_.Initialize(device, logStream, isInstancing);

	/// --- InputLayout ---

	// 頂点データの並びを設定
	inputLayout_.Initialize();

	/// --- RasterizerState ---

	// 面の判定と塗りつぶし方法を設定
	rasterizerState_.Initialize();

	/// --- ShaderProgram ---

	// 借りたDXCで、指定されたVertexShaderとPixelShaderをコンパイル
	shaderProgram_.Initialize(dxcCompiler, logStream, vertexShaderPath, pixelShaderPath);

	/// --- DepthStencilState ---

	// 奥行きの比較と書き込み方法を設定 (半透明で重なるParticle用は、Depthを書き込まない)
	depthStencilState_.Initialize(isDepthWriteEnabled);

	/// --- GraphicsPipelineState ---

	// BlendMode以外の共通設定をまとめる
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature_.Get(); // RootSignature
	graphicsPipelineStateDesc.InputLayout = inputLayout_.GetDesc(); // InputLayout
	graphicsPipelineStateDesc.VS = shaderProgram_.GetVertexShaderByteCode(); // VertexShader
	graphicsPipelineStateDesc.PS = shaderProgram_.GetPixelShaderByteCode(); // PixelShader
	graphicsPipelineStateDesc.RasterizerState = rasterizerState_.GetDesc(); // RasterizerState
	// DepthStencilの設定、Depthを使いますよという意思表示
	graphicsPipelineStateDesc.DepthStencilState = depthStencilState_.GetDesc();
	graphicsPipelineStateDesc.DSVFormat = depthStencilState_.GetFormat();

	// 書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するトロポジ(形状)のタイプ、三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// どのように画面に色を打ち込むかの設定
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	/// --- BlendModeごとのPSO ---

	// BlendStateだけを差し替えて、BlendModeの数だけPSOを生成する
	for (size_t i = 0; i < graphicsPipelineStates_.size(); ++i) {
		// BlendModeに合わせて色の書き込み方法を設定
		BlendState blendState;
		blendState.Initialize(static_cast<BlendMode>(i));
		graphicsPipelineStateDesc.BlendState = blendState.GetDesc();

		// 実際に生成
		HRESULT hr = device->CreateGraphicsPipelineState(
			&graphicsPipelineStateDesc,
			IID_PPV_ARGS(graphicsPipelineStates_[i].GetAddressOf())
		);
		assert(SUCCEEDED(hr));
	}
}

/// --- 終了処理 ---
// PSO関連のリソースを解放
void PipelineState::Finalize() {
	// PipelineStateはComPtrのデストラクタが自動解放する
	shaderProgram_.Finalize();
	rootSignature_.Finalize();
}
