#pragma once

#include "CommandContext.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <ostream>
#include <string>
#include <array>

// ポインタしか使わないので前方宣言
class Window;

/// <summary>
/// DirectX基盤
/// </summary>
class GraphicsDevice {
public:
	///// ----- メンバ関数 ----- /////

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(Window* window, std::ostream& logStream);

	/// <summary>
	/// 終了
	/// </summary>
	void Finalize();

	///// ----- ===== ----- /////

	///// ----- 定数 ----- /////

	// バックバッファの枚数 (ダブルバッファ)
	static const UINT kBackBufferCount = 2;

	///// ----- ===== ----- /////

	///// ----- getter ----- /////

	/// <summary>
	/// デバイス
	/// </summary>
	/// <returns></returns>
	ID3D12Device* GetDevice() const {
		return device_.Get();
	}

	/// <summary>
	/// DXGIファクトリー
	/// </summary>
	/// <returns></returns>
	IDXGIFactory7* GetDxgiFactory() const {
		return dxgiFactory_.Get();
	}

	/// <summary>
	/// GPUの名前
	/// </summary>
	/// <returns></returns>
	const std::string& GetAdapterName() const {
		return adapterName_;
	}

	/// <summary>
	/// コマンドの記録と実行を管理
	/// </summary>
	/// <returns></returns>
	CommandContext* GetCommandContext() {
		return &commandContext_;
	}

	/// <summary>
	/// コマンドリスト
	/// </summary>
	/// <returns></returns>
	ID3D12GraphicsCommandList* GetCommandList() const {
		return commandContext_.GetCommandList();
	}

	/// <summary>
	/// スワップチェーン
	/// </summary>
	IDXGISwapChain4* GetSwapChain() const {
		return swapChain_.Get();
	}

	/// <summary>
	/// スワップチェーンの設定
	/// </summary>
	const DXGI_SWAP_CHAIN_DESC1& GetSwapChainDesc() const {
		return swapChainDesc_;
	}

	/// <summary>
	/// バックバッファ (リサイズで作り直すので書き換えられる参照で返す)
	/// </summary>
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, kBackBufferCount>& GetSwapChainResources() {
		return swapChainResources_;
	}

	///// ----- ===== ----- /////

private:
	///// ----- メンバ関数 ----- /////

	/// <summary>
	/// デバイスの初期化
	/// </summary>
	/// <param name="logStream"></param>
	void InitializeDevice(std::ostream& logStream);

	/// <summary>
	/// コマンド関連の初期化 (CommandQueue Allocator List Fence)
	/// </summary>
	void InitializeCommand();

	/// <summary>
	/// スワップチェーンの生成とバックバッファの取得
	/// </summary>
	void InitializeSwapChain();

	///// ----- ===== ----- /////

	///// ----- メンバ変数 ----- /////

	// 生成で使うウィンドウ 借りるだけ
	Window* window_ = nullptr;

	// DirectX12デバイス
	Microsoft::WRL::ComPtr<ID3D12Device> device_;

	// DXGIファクトリー
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_;

	// 使用しているGPUの名前 (画面表示用)
	std::string adapterName_;

	// コマンドの記録と実行を管理 (Fenceも含む)
	CommandContext commandContext_;

	// スワップチェーン
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_;

	// スワップチェーンの設定 (リサイズやImGuiの初期化でも使う)
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc_{};

	// バックバッファ (スワップチェーンのリソース)
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, kBackBufferCount> swapChainResources_;

	///// ----- ===== ----- /////
};

