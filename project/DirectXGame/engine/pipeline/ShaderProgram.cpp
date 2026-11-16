#include "ShaderProgram.h"
#include "DxcCompiler.h"

#include <cassert>

///// ----- ShaderProgram ----- /////

ShaderProgram::~ShaderProgram() {
	Finalize();
}

/// --- 初期化 ---
// 借りたDXCで、指定されたVertexShaderとPixelShaderをコンパイル
void ShaderProgram::Initialize(
	DxcCompiler* dxcCompiler,
	std::ostream& logStream,
	const std::wstring& vertexShaderPath,
	const std::wstring& pixelShaderPath) {

	// NULL検出
	assert(dxcCompiler);

	// 指定されたShaderをCompileする
	vertexShaderBlob_ = dxcCompiler->CompileShader(vertexShaderPath, L"vs_6_0", logStream);
	assert(vertexShaderBlob_ != nullptr);

	pixelShaderBlob_ = dxcCompiler->CompileShader(pixelShaderPath, L"ps_6_0", logStream);
	assert(pixelShaderBlob_ != nullptr);
}

/// --- 取得 ---
// VertexShaderのByteCodeを取得
D3D12_SHADER_BYTECODE ShaderProgram::GetVertexShaderByteCode() const {
	return {vertexShaderBlob_->GetBufferPointer(), vertexShaderBlob_->GetBufferSize()};
}

// PixelShaderのByteCodeを取得
D3D12_SHADER_BYTECODE ShaderProgram::GetPixelShaderByteCode() const {
	return {pixelShaderBlob_->GetBufferPointer(), pixelShaderBlob_->GetBufferSize()};
}

/// --- 終了処理 ---
// Shaderで使用したリソースを解放
void ShaderProgram::Finalize() {

	// ShaderBlobはComPtrのデストラクタが自動解放する
}
