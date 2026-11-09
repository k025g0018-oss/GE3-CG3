#pragma once

#include "CommandContext.h"
#include "DescriptorHeap.h"
#include "DepthStencilView.h"
#include "DxcCompiler.h"
#include "Window.h"
#include "BufferResource.h"
#include "FrameRateController.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <ostream>
#include <string>
#include <array>
#include <vector>
#include <cassert>

// 前方宣言
class SceneRenderTexture;

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

	/// <summary>
	/// ウィンドウの大きさに合わせてバックバッファを作り直す
	/// </summary>
	/// <returns>作り直したらtrue (大きさが同じなら何もしないでfalse)</returns>
	bool Resize(uint32_t width, uint32_t height);

	/// <summary>
	/// 描画前処理 (バックバッファを描画できる状態に整える)
	/// </summary>
	void PreDraw();

	/// <summary>
	/// 描画後処理 (コマンドをGPUに送って画面を更新する)
	/// </summary>
	void PostDraw();

	/// <summary>
	/// バッファリソースの生成 (デバイスは自分のものを使う)
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeInBytes) const {
		return BufferResource::Create(device_.Get(), sizeInBytes);
	}

#ifdef USE_IMGUI

	/// <summary>
	/// ImGuiのフレーム開始 (毎フレームの最初に呼ぶ)
	/// </summary>
	void BeginImGuiFrame();

	/// <summary>
	/// ImGuiの描画コマンドを積む
	/// </summary>
	void DrawImGui();

	/// <summary>
	/// ゲーム画面の描画先にするScene用テクスチャを設定する 借りるだけ
	/// </summary>
	void SetSceneRenderTexture(SceneRenderTexture* sceneRenderTexture) {
		sceneRenderTexture_ = sceneRenderTexture;
	}

#endif // USE_IMGUI

	///// ----- ===== ----- /////

	///// ----- 定数 ----- /////

	// バックバッファの枚数 (ダブルバッファ)
	static const UINT kBackBufferCount = 2;

	// SRV用ディスクリプタヒープに確保する数 (テクスチャとImGuiで使う)
	static const UINT kSrvDescriptorCount = 128;

	// ImGuiに割り当てるSRVの開始番号 (ゲーム用SRVと重ならない64番以降)
	static const UINT kImGuiSrvStartIndex = 64;

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
	/// バックバッファ
	/// </summary>
	const std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, kBackBufferCount>& GetSwapChainResources() const {
		return swapChainResources_;
	}

	/// <summary>
	/// SRV用ディスクリプタヒープ
	/// </summary>
	DescriptorHeap& GetSrvDescriptorHeap() {
		return srvDescriptorHeap_;
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

	/// <summary>
	/// バックバッファ全体のビューポート (ImGuiを描くときに使う)
	/// </summary>
	const D3D12_VIEWPORT& GetBackBufferViewport() const {
		return backBufferViewport_;
	}

	/// <summary>
	/// バックバッファ全体のシザー矩形 (ImGuiを描くときに使う)
	/// </summary>
	const D3D12_RECT& GetBackBufferScissorRect() const {
		return backBufferScissorRect_;
	}

	/// <summary>
	/// SRVの指定番号のCPUデスクリプタハンドルを取得する
	/// </summary>
	D3D12_CPU_DESCRIPTOR_HANDLE GetSrvCpuHandle(uint32_t index) const {
		return srvDescriptorHeap_.GetCPUHandle(index);
	}

	/// <summary>
	/// SRVの指定番号のGPUデスクリプタハンドルを取得する
	/// </summary>
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvGpuHandle(uint32_t index) const {
		return srvDescriptorHeap_.GetGPUHandle(index);
	}

	/// <summary>
	/// フレームレートの管理 (deltaTimeの取得やFPS固定の切り替え)
	/// </summary>
	FrameRateController* GetFrameRateController() {
		return &frameRateController_;
	}

	///// ----- ===== ----- /////

private:
#ifdef USE_IMGUI

	///// ----- ImGui用SRVの割り当て ----- /////

	// Dear ImGui 1.92以降が必要とするSRVの確保と解放を管理する
	struct ImGuiSrvDescriptorAllocator {
		const DescriptorHeap* descriptorHeap = nullptr;
		std::vector<uint32_t> freeIndices;

		// ゲーム用SRVと衝突しないようにヒープ後半をImGui専用にする
		void Initialize(const DescriptorHeap& heap, uint32_t firstIndex) {

			descriptorHeap = &heap;
			// 使える番号を、後ろから順に空きリストへ入れておく
			for (uint32_t index = heap.GetDescriptorCount(); index > firstIndex; --index) {
				freeIndices.push_back(index - 1);
			}
		}

		// 空いている番号を1つ取り出して、その場所のハンドルを返す
		void Allocate(D3D12_CPU_DESCRIPTOR_HANDLE* cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE* gpuHandle) {
			assert(!freeIndices.empty());
			const uint32_t index = freeIndices.back();
			freeIndices.pop_back();
			*cpuHandle = descriptorHeap->GetCPUHandle(index);
			*gpuHandle = descriptorHeap->GetGPUHandle(index);
		}

		// 使い終わったハンドルから番号を計算して、空きリストへ戻す
		void Free(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle) {
			const SIZE_T cpuOffset = cpuHandle.ptr - descriptorHeap->GetCPUHandleStart().ptr;
			const UINT64 gpuOffset = gpuHandle.ptr - descriptorHeap->GetGPUHandleStart().ptr;
			const uint32_t cpuIndex = static_cast<uint32_t>(cpuOffset / descriptorHeap->GetDescriptorSize());
			const uint32_t gpuIndex = static_cast<uint32_t>(gpuOffset / descriptorHeap->GetDescriptorSize());
			assert(cpuIndex == gpuIndex);
			freeIndices.push_back(cpuIndex);
		}
	};

	///// ----- ===== ----- /////

#endif // USE_IMGUI

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

#ifdef USE_IMGUI

	/// <summary>
	/// ImGuiの初期化
	/// </summary>
	void InitializeImGui();

	/// <summary>
	/// ImGuiの終了
	/// </summary>
	void FinalizeImGui();

#endif // USE_IMGUI

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

	// 今のバックバッファの大きさ (リサイズが必要か判定する)
	uint32_t backBufferWidth_ = Window::kClientWidth;
	uint32_t backBufferHeight_ = Window::kClientHeight;

	// バックバッファ全体のビューポート (ウィンドウの大きさに追従する)
	D3D12_VIEWPORT backBufferViewport_{};

	// バックバッファ全体のシザー矩形 (ウィンドウの大きさに追従する)
	D3D12_RECT backBufferScissorRect_{};

	// フレームレートの管理 (FPS固定)
	FrameRateController frameRateController_;

#ifdef USE_IMGUI

	// ImGuiに渡すSRVの割り当て係 (ImGuiが使っている間は消えてはいけない)
	ImGuiSrvDescriptorAllocator imguiSrvAllocator_;

	// ゲーム画面の描画先 (ImGuiのSceneパネル用) 借りるだけ
	SceneRenderTexture* sceneRenderTexture_ = nullptr;

#endif // USE_IMGUI

	///// ----- ===== ----- /////
};

