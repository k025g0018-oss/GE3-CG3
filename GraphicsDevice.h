#pragma once

#include "CommandContext.h"
#include "DescriptorHeap.h"
#include "DepthStencilView.h"
#include "DxcCompiler.h"

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

	// SRV用ディスクリプタヒープに確保する数 (テクスチャとImGuiで使う)
	static const UINT kSrvDescriptorCount = 128;

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


	/// <summary>
	/// SRV用ディスクリプタヒープ
	/// </summary>
	DescriptorHeap& GetSrvDescriptorHeap() {
		return srvDescriptorHeap_;
	}

	/// <summary>
	/// DSV用ディスクリプタヒープ
	/// </summary>
	DescriptorHeap& GetDsvDescriptorHeap() {
		return dsvDescriptorHeap_;
	}

	/// <summary>
	/// RTVの設定 (フォーマットはImGuiやSceneRenderTextureでも使う)
	/// </summary>
	const D3D12_RENDER_TARGET_VIEW_DESC& GetRtvDesc() const {
		return rtvDesc_;
	}

	/// <summary>
	/// バックバッファごとのRTVのハンドル
	/// </summary>
	const std::array<D3D12_CPU_DESCRIPTOR_HANDLE, kBackBufferCount>& GetRtvHandles() const {
		return rtvHandles_;
	}

	/// <summary>
	/// 深度ステンシルビューのハンドル
	/// </summary>
	D3D12_CPU_DESCRIPTOR_HANDLE GetDsvHandle() const {
		return depthStencilView_.GetHandle();
	}

	/// <summary>
	/// ゲーム画面用のビューポート
	/// </summary>
	const D3D12_VIEWPORT& GetViewport() const {
		return viewport_;
	}

	/// <summary>
	/// ゲーム画面用のシザー矩形
	/// </summary>
	const D3D12_RECT& GetScissorRect() const {
		return scissorRect_;
	}

	/// <summary>
	/// シェーダーのコンパイラ (PSOを作るときに借りる)
	/// </summary>
	DxcCompiler* GetDxcCompiler() {
		return &dxcCompiler_;
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

	/// <summary>
	/// 各種ディスクリプタヒープの生成 (RTV・SRV・DSV)
	/// </summary>
	void InitializeDescriptorHeaps();

	/// <summary>
	/// レンダーターゲットビューの生成
	/// </summary>
	void InitializeRenderTargetView();

	/// <summary>
	/// 深度バッファと深度ステンシルビューの生成
	/// </summary>
	void InitializeDepthStencilView();

	/// <summary>
	/// ビューポートの初期化
	/// </summary>
	void InitializeViewport();

	/// <summary>
	/// シザー矩形の初期化
	/// </summary>
	void InitializeScissorRect();

	/// <summary>
	/// DXCコンパイラの初期化
	/// </summary>
	void InitializeDxcCompiler();

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

	// RTV用ディスクリプタヒープ (バックバッファの数だけ)
	DescriptorHeap rtvDescriptorHeap_;

	// SRV用ディスクリプタヒープ (テクスチャとImGuiで使う)
	DescriptorHeap srvDescriptorHeap_;

	// DSV用ディスクリプタヒープ (深度バッファ1つ分)
	DescriptorHeap dsvDescriptorHeap_;

	// RTVの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc_{};

	// バックバッファごとのRTVのハンドル
	std::array<D3D12_CPU_DESCRIPTOR_HANDLE, kBackBufferCount> rtvHandles_{};

	// 深度バッファと深度ステンシルビュー
	DepthStencilView depthStencilView_;

	// ゲーム画面用のビューポート (クライアント領域と同じ大きさ)
	D3D12_VIEWPORT viewport_{};

	// ゲーム画面用のシザー矩形 (ビューポートと同じ大きさ)
	D3D12_RECT scissorRect_{};

	// シェーダーのコンパイラ (1つをみんなで使う)
	DxcCompiler dxcCompiler_;

	///// ----- ===== ----- /////
};

