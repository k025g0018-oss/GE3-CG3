#include "GraphicsDevice.h"
#include "Window.h"
#include "Logger.h"

#include <cassert>
#include <format>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib, "dxgi.lib")

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


}

/// <summary>
/// 終了
/// </summary>
void GraphicsDevice::Finalize() {

	// コマンド関連を開放する
	commandContext_.Finalize();

	// ComPtrなので、デストラクタで自動解放される
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
			Log(logStream, std::format(L"Use Adapter:{}\n", adapterDesc.Description));
			// 画面に表示するため、GPUの名前をUTF-8の文字列にして記録する
			adapterName_ = ConvertString(adapterDesc.Description);

			break;
		}
		useAdapter = nullptr; // ソフトウェアアダプタの場合は見なかったことにする
	}

	// 適切なアダプタが見つからなかったので起動できない
	assert(useAdapter != nullptr);

	// --- D3D12Deviceの生成 ---
	// 
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
			Log(logStream, std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
			break; // 生成できたらループを抜ける
		}
	}

	// デバイスの生成がうまくいかなかったので起動できない
	assert(device_ != nullptr);
	Log(logStream, "Complete create D3D12Device!!!\n"); // 初期化完了のログを出す

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