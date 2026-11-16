#pragma once

#include <d3d12.h>
#include <dxcapi.h>
#include <ostream>
#include <string>
#include <wrl.h>

// 前方宣言
class DxcCompiler;

///// ----- ShaderProgram ----- /////

class ShaderProgram {
public:
	ShaderProgram() = default;
	~ShaderProgram();

	ShaderProgram(const ShaderProgram&) = delete;
	ShaderProgram& operator=(const ShaderProgram&) = delete;

	/// --- 初期化 ---
	// 借りたDXCで、指定されたVertexShaderとPixelShaderをコンパイル
	void Initialize(
		DxcCompiler* dxcCompiler,
		std::ostream& logStream,
		const std::wstring& vertexShaderPath,
		const std::wstring& pixelShaderPath);

	/// --- 取得 ---
	// VertexShaderのByteCodeを取得
	D3D12_SHADER_BYTECODE GetVertexShaderByteCode() const;

	// PixelShaderのByteCodeを取得
	D3D12_SHADER_BYTECODE GetPixelShaderByteCode() const;

	/// --- 終了処理 ---
	// Shaderで使用したリソースを解放
	void Finalize();

private:
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob_;
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob_;
};
