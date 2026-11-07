#include "GraphicsDevice.h"
#include "Window.h"
#include "Logger.h"
#include "StringUtility.h"
#include "SceneRenderTexture.h"

#include <cassert>
#include <format>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#ifdef USE_IMGUI

#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"

#endif // USE_IMGUI

using Microsoft::WRL::ComPtr;

/// <summary>
/// 初期化
/// </summary>
void GraphicsDevice::Initialize(Window* window, std::ostream& logStream) {

	// Null検出
	assert(window);

	// 借りてきたWindowのインスタンスを記録
	window_ = window;

	// デバイスの初期化 (ID3D12Device)
	InitializeDevice(logStream);

	// コマンド関連の初期化
	InitializeCommand();

	// スワップチェーンの生成
	InitializeSwapChain();

	// 各種ディスクリプタヒープの生成
	InitializeDescriptorHeaps();

	// レンダーターゲットビューの生成
	InitializeRenderTargetView();

	// 深度バッファと深度ステンシルビューの生成
	InitializeDepthStencilView();

	// ビューポートの初期化
	InitializeViewport();

	// シザー矩形の初期化
	InitializeScissorRect();

	// DXCコンパイラの初期化
	InitializeDxcCompiler();

#ifdef USE_IMGUI

	// ImGuiの初期化
	InitializeImGui();

#endif // USE_IMGUI
}

/// <summary>
/// 終了
/// </summary>
void GraphicsDevice::Finalize() {

#ifdef USE_IMGUI

	// ImGuiを終了する
	FinalizeImGui();

#endif // USE_IMGUI

	// DXCコンパイラを解放する
	dxcCompiler_.Finalize();

	// 深度バッファを解放する
	depthStencilView_.Finalize();

	// 各種ディスクリプタヒープを解放する
	rtvDescriptorHeap_.Finalize();
	srvDescriptorHeap_.Finalize();
	dsvDescriptorHeap_.Finalize();

	// コマンド関連を開放する
	commandContext_.Finalize();

	// ComPtrなので、デストラクタで自動解放される
}

/// <summary>
/// 描画前処理
/// </summary>
void GraphicsDevice::PreDraw() {

	ID3D12GraphicsCommandList* commandList = commandContext_.GetCommandList();

	/// --- バックバッファの番号取得 ---
	UINT backBufferIndex = swapChain_->GetCurrentBackBufferIndex();

	/// --- リソースバリアで書き込み可能に変更 ---
	D3D12_RESOURCE_BARRIER barrier{};
	// 今回のバリアはTransition
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	// 現在のバックバッファに対して行う
	barrier.Transition.pResource = swapChainResources_[backBufferIndex].Get();
	// 表示状態 → 描画状態
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	commandList->ResourceBarrier(1, &barrier);

	/// --- 描画先のRTVとDSVを指定する ---
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = depthStencilView_.GetHandle();
	D3D12_CPU_DESCRIPTOR_HANDLE gameRenderTarget = rtvHandles_[backBufferIndex];

#ifdef USE_IMGUI

	// エディター有効時はゲームをScene用テクスチャへ描画する
	if (sceneRenderTexture_) {
		sceneRenderTexture_->TransitionToRenderTarget(commandList);
		gameRenderTarget = sceneRenderTexture_->GetRTVHandle();
	}

#endif // USE_IMGUI

	commandList->OMSetRenderTargets(1, &gameRenderTarget, false, &dsvHandle);

	/// --- 画面全体の色をクリア ---
	const float clearColor[] = {0.1f, 0.25f, 0.5f, 1.0f}; // 青っぽい色。RGBAの順
	commandList->ClearRenderTargetView(gameRenderTarget, clearColor, 0, nullptr);

	/// --- 画面全体の深度をクリア ---
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	/// --- SRV用のデスクリプタヒープを指定する ---
	ID3D12DescriptorHeap* descriptorHeaps[] = {srvDescriptorHeap_.Get()};
	commandList->SetDescriptorHeaps(1, descriptorHeaps);

	/// --- ビューポート領域の設定 ---
	commandList->RSSetViewports(1, &viewport_);

	/// --- シザー矩形の設定 ---
	commandList->RSSetScissorRects(1, &scissorRect_);
}

/// <summary>
/// 描画後処理
/// </summary>
void GraphicsDevice::PostDraw() {
}

/// <summary>
/// デバイスの初期化
/// </summary>
/// <param name="logStream"></param>
void GraphicsDevice::InitializeDevice(std::ostream& logStream) {

	HRESULT hr;

	/// --- DebugLayer ---

#ifdef _DEBUG
	ComPtr<ID3D12Debug1> debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(debugController.GetAddressOf())))) {
		// デバッグレイヤーを有効化する
		debugController->EnableDebugLayer();
		// さらにGPU側でもチェックを行うようにする
		debugController->SetEnableGPUBasedValidation(TRUE);
	}
#endif // _DEBUG

	/// --- DXGIファクトリーの生成 ---

	// HRESULTはWindowsケイのエラーコード、関数が成功したかどうかをSUCCEEDEDマクロで判定できる
	hr = CreateDXGIFactory(IID_PPV_ARGS(dxgiFactory_.GetAddressOf()));
	// 初期化の根本的な部分でエラーが出た場合はプログラムが間違っているか、どうにもできない場合はassertにしておく
	assert(SUCCEEDED(hr));

	/// --- アダプターの選択 ---

	// 使用するアダプタ用の変数、最初にnullptr
	ComPtr<IDXGIAdapter4> useAdapter;

	// 良い順にアダプタを頼む
	for (UINT i = 0; dxgiFactory_->EnumAdapterByGpuPreference(
		i,
		DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
		IID_PPV_ARGS(useAdapter.ReleaseAndGetAddressOf())) != DXGI_ERROR_NOT_FOUND;
		++i) {
		// アダプターの情報を取得する
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr)); // 取得できないのは一大事

		// ソフトウェアアダプタでなければ採用
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			// 採用したアダプタの情報をログに出力。wstringの方なので注意
			Logger::Log(logStream, std::format(L"Use Adapter:{}\n", adapterDesc.Description));
			// 画面に表示するため、GPUの名前をUTF-8の文字列にして記録する
			adapterName_ = StringUtility::ConvertString(adapterDesc.Description);

			break;
		}
		useAdapter = nullptr; // ソフトウェアアダプタの場合は見なかったことにする
	}

	// 適切なアダプタが見つからなかったので起動できない
	assert(useAdapter != nullptr);

	// --- D3D12Deviceの生成 ---

	// 機能レベルとログ出力用の文字列
	D3D_FEATURE_LEVEL featureLevels[]{
		D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0};
	const char* featureLevelStrings[] = {"12.2", "12.1", "12.0"};
	// 高い順に生成できるか試していく
	for (size_t i = 0; i < _countof(featureLevels); ++i) {
		// 採用したアダプターでデバイスを生成
		hr = D3D12CreateDevice(
			useAdapter.Get(),
			featureLevels[i],
			IID_PPV_ARGS(device_.ReleaseAndGetAddressOf()));
		// 指定した機能レベルでデバイスが生成できたかを確認
		if (SUCCEEDED(hr)) {
			// 生成できたのでログ出力を行ってループを抜ける
			Logger::Log(logStream, std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
			break; // 生成できたらループを抜ける
		}
	}

	// デバイスの生成がうまくいかなかったので起動できない
	assert(device_ != nullptr);
	Logger::Log(logStream, "Complete create D3D12Device!!!\n"); // 初期化完了のログを出す

	// エラー・警告を実行時にプログラムを停止させる、deviceに対して行う
#ifdef _DEBUG
	ComPtr<ID3D12InfoQueue> infoQueue;
	if (SUCCEEDED(device_->QueryInterface(IID_PPV_ARGS(infoQueue.GetAddressOf())))) {
		// やばいエラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		// エラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		// 警告時に止まる(ここをコメントアウトしたら全部の情報が出力される、詳細な情報をログに出力することができる)
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

		// エラーと警告の抑制
		// 抑制するメッセージのID
		D3D12_MESSAGE_ID denyIds[] = {
			// windows11でのDXGIデバッグレイヤーとDX12デバッグレイヤーの相互作用バクによるエラーメッセージ
			// https://stackoverflow.com/questions/69805245/directx-12-application-is-crashing-in-windows-11
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE};

		// 抑制するレベル
		D3D12_MESSAGE_SEVERITY severities[] = {D3D12_MESSAGE_SEVERITY_INFO};
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;
		// 指定したメッセージの表示を抑制する
		infoQueue->PushStorageFilter(&filter);

		// 解放
		// infoQueueはComPtrが自動解放する
	}
#endif // _DEBUG

}

/// <summary>
/// コマンド関連の初期化
/// </summary>
void GraphicsDevice::InitializeCommand() {

	// CommandQueue・CommandAllocator・CommandList・Fenceを生成する
	commandContext_.Initialize(device_.Get());
}

/// <summary>
/// スワップチェーンの生成とバックバッファの取得
/// </summary>
void GraphicsDevice::InitializeSwapChain() {

	HRESULT hr;

	/// --- SwapChainの生成 ---

	swapChainDesc_.Width = Window::kClientWidth;					 // 画面の幅。ウィンドウのクライアント領域を同じものにしておく。
	swapChainDesc_.Height = Window::kClientHeight;					 // 画面の高さ。ウィンドウのクライアント領域を同じものにしておく。
	swapChainDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM;				 // 色の形式
	swapChainDesc_.SampleDesc.Count = 1;							 // マルチサンプルしない
	swapChainDesc_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	 // 描画のターゲットとして利用する
	swapChainDesc_.BufferCount = kBackBufferCount;					 // ダブルバッファ
	swapChainDesc_.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;		 // モニターにうつしたら、中身を廃棄

	// コマンドキュー、ウィンドウハンドル、設定を渡して生成する。
	hr = dxgiFactory_->CreateSwapChainForHwnd(
		commandContext_.GetCommandQueue(),
		window_->GetHwnd(),
		&swapChainDesc_,
		nullptr,
		nullptr,
		reinterpret_cast<IDXGISwapChain1**>(swapChain_.GetAddressOf()));
	assert(SUCCEEDED(hr));

	/// --- SwapChainからResourceを引っ張ってくる ---

	for (UINT index = 0; index < kBackBufferCount; ++index) {
		hr = swapChain_->GetBuffer(index, IID_PPV_ARGS(swapChainResources_[index].GetAddressOf()));
		assert(SUCCEEDED(hr));
	}
}

/// <summary>
/// 各種ディスクリプタヒープの生成
/// </summary>
void GraphicsDevice::InitializeDescriptorHeaps() {

	// RTV用のヒープ。バックバッファの数だけ作る。RTVはShader内で触るものではないので、ShaderVisibleはfalse
	rtvDescriptorHeap_.Initialize(device_.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, kBackBufferCount, false);

	// SRV用のヒープ。SRVはShader内で触るものなので、ShaderVisibleはtrue
	srvDescriptorHeap_.Initialize(device_.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kSrvDescriptorCount, true);

	// DSV用のヒープ。深度バッファは1つ。DSVはShader内で触るものではないので、ShaderVisibleはfalse
	dsvDescriptorHeap_.Initialize(device_.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);
}

/// <summary>
/// レンダーターゲットビューの生成
/// </summary>
void GraphicsDevice::InitializeRenderTargetView() {

	// RTVの設定
	rtvDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;	   // 出力結果をSRGBに変換して書き込む
	rtvDesc_.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D; // 2dテクスチャとして書き込む

	// バックバッファの数だけRTVを作る
	for (UINT index = 0; index < kBackBufferCount; ++index) {
		// RTV用ヒープのindex番目の場所を取得する
		rtvHandles_[index] = rtvDescriptorHeap_.GetCPUHandle(index);
		// その場所に、バックバッファのRTVを作る
		device_->CreateRenderTargetView(swapChainResources_[index].Get(), &rtvDesc_, rtvHandles_[index]);
	}
}

/// <summary>
/// 深度バッファと深度ステンシルビューの生成
/// </summary>
void GraphicsDevice::InitializeDepthStencilView() {

	// クライアント領域と同じ大きさの深度バッファを作り、DSV用ヒープにDSVを作る
	depthStencilView_.Initialize(device_.Get(), dsvDescriptorHeap_, Window::kClientWidth, Window::kClientHeight);
}

/// <summary>
/// ビューポートの初期化
/// </summary>
void GraphicsDevice::InitializeViewport() {

	// クライアント領域のサイズと一緒にして画面全体に表示
	viewport_.Width = static_cast<float>(Window::kClientWidth);
	viewport_.Height = static_cast<float>(Window::kClientHeight);
	viewport_.TopLeftX = 0.0f;
	viewport_.TopLeftY = 0.0f;
	viewport_.MinDepth = 0.0f;
	viewport_.MaxDepth = 1.0f;

	// バックバッファ全体用も、最初はゲーム画面と同じ大きさにしておく
	backBufferViewport_ = viewport_;
}

/// <summary>
/// シザー矩形の初期化
/// </summary>
void GraphicsDevice::InitializeScissorRect() {

	// ビューポートと同じ矩形が構成されるようにする
	scissorRect_.left = 0;
	scissorRect_.right = Window::kClientWidth;
	scissorRect_.top = 0;
	scissorRect_.bottom = Window::kClientHeight;

	// バックバッファ全体用も、最初はゲーム画面と同じ大きさにしておく
	backBufferScissorRect_ = scissorRect_;
}

/// <summary>
/// DXCコンパイラの初期化
/// </summary>
void GraphicsDevice::InitializeDxcCompiler() {

	// DxcUtils・DxcCompiler・IncludeHandlerを生成する
	dxcCompiler_.Initialize();
}

/// <summary>
/// ウィンドウの大きさに合わせてバックバッファを作り直す
/// </summary>
bool GraphicsDevice::Resize(uint32_t width, uint32_t height) {

	// 大きさが変わっていなければ何もしない
	if (width == backBufferWidth_ && height == backBufferHeight_) {
		return false;
	}

	// ResizeBuffers前にGPUのBackBuffer参照を完了させる
	commandContext_.ExecuteAndWait();

	// ResizeBuffersの前にBackBufferの参照を解放する
	for (Microsoft::WRL::ComPtr<ID3D12Resource>& backBuffer : swapChainResources_) {
		backBuffer.Reset();
	}

	// バックバッファを新しい大きさで作り直す
	HRESULT hr = swapChain_->ResizeBuffers(
		swapChainDesc_.BufferCount,
		width,
		height,
		swapChainDesc_.Format,
		0);
	assert(SUCCEEDED(hr));

	// 作り直したバックバッファを取得して、RTVも作り直す
	for (UINT index = 0; index < kBackBufferCount; ++index) {
		hr = swapChain_->GetBuffer(index, IID_PPV_ARGS(swapChainResources_[index].GetAddressOf()));
		assert(SUCCEEDED(hr));
		device_->CreateRenderTargetView(swapChainResources_[index].Get(), &rtvDesc_, rtvHandles_[index]);
	}

	// 新しい大きさを記録する
	backBufferWidth_ = width;
	backBufferHeight_ = height;

	// バックバッファ全体のビューポートとシザー矩形も新しい大きさにする
	backBufferViewport_.Width = static_cast<float>(width);
	backBufferViewport_.Height = static_cast<float>(height);
	backBufferScissorRect_.right = static_cast<LONG>(width);
	backBufferScissorRect_.bottom = static_cast<LONG>(height);

	return true;
}

#ifdef USE_IMGUI

/// <summary>
/// ImGuiの初期化
/// </summary>
void GraphicsDevice::InitializeImGui() {

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();

	// Docking対応版へ更新した後に有効化する
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(window_->GetHwnd());

	// Dear ImGuiのDirectX 12初期化情報をまとめる
	ImGui_ImplDX12_InitInfo initInfo{};
	initInfo.Device = device_.Get();
	initInfo.CommandQueue = commandContext_.GetCommandQueue();
	initInfo.NumFramesInFlight = swapChainDesc_.BufferCount;
	initInfo.RTVFormat = rtvDesc_.Format;
	initInfo.DSVFormat = DXGI_FORMAT_UNKNOWN;
	initInfo.SrvDescriptorHeap = srvDescriptorHeap_.Get();

	// Dear ImGuiにはゲーム用SRVと重ならない番号以降を割り当てる
	imguiSrvAllocator_.Initialize(srvDescriptorHeap_, kImGuiSrvStartIndex);
	initInfo.UserData = &imguiSrvAllocator_;
	initInfo.SrvDescriptorAllocFn = [](
		ImGui_ImplDX12_InitInfo* info,
		D3D12_CPU_DESCRIPTOR_HANDLE* cpuHandle,
		D3D12_GPU_DESCRIPTOR_HANDLE* gpuHandle
		) {
			auto* allocator = static_cast<ImGuiSrvDescriptorAllocator*>(info->UserData);
			allocator->Allocate(cpuHandle, gpuHandle);
		};
	initInfo.SrvDescriptorFreeFn = [](
		ImGui_ImplDX12_InitInfo* info,
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle
		) {
			auto* allocator = static_cast<ImGuiSrvDescriptorAllocator*>(info->UserData);
			allocator->Free(cpuHandle, gpuHandle);
		};

	// 新しいDirectX 12バックエンド初期化形式を使用する
	ImGui_ImplDX12_Init(&initInfo);

	io.Fonts->Build();
}

/// <summary>
/// ImGuiの終了
/// </summary>
void GraphicsDevice::FinalizeImGui() {
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

/// <summary>
/// ImGuiのフレーム開始
/// </summary>
void GraphicsDevice::BeginImGuiFrame() {
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

/// <summary>
/// ImGuiの描画コマンドを積む
/// </summary>
void GraphicsDevice::DrawImGui() {
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandContext_.GetCommandList());
}

#endif // USE_IMGUI