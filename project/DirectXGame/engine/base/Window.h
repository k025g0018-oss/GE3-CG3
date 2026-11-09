#pragma once

#include <Windows.h>
#include <cstdint>

/// <summary>
/// WindowsAPI
/// ウィンドウの生成と管理
/// </summary>
class Window {
public:
	///// ----- 静的メンバ関数 ----- /////

	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

	///// ----- ===== ----- /////

	///// ----- メンバ関数 ----- /////

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 終了
	/// </summary>
	void Finalize();

	/// <summary>
	/// メッセージの処理
	/// </summary>
	/// <returns></returns>
	bool ProcessMessage();

	///// ----- ===== ----- /////

	///// ----- 定数 ----- /////

	/// --- クライアント領域のサイズ ---
	static const int32_t kClientWidth = 1280;
	static const int32_t kClientHeight = 720;

	///// ----- ===== ----- /////

	///// ----- getter ----- /////

	/// <summary>
	/// HWND
	/// </summary>
	/// <returns></returns>
	HWND GetHwnd()const {
		return hwnd_;
	}

	/// <summary>
	/// HINSTANCE
	/// </summary>
	/// <returns></returns>
	HINSTANCE GetHInstance() const {
		return wc_.hInstance;
	}

	///// ----- ===== ----- /////

	///// ----- setter ----- /////

	///// ----- ===== ----- /////

private:
	///// ----- 関数 ----- /////


	///// ----- ===== ----- /////

	///// ----- メンバ変数 ----- /////

	// ウィンドウハンドル
	HWND hwnd_ = nullptr;

	// ウィンドウクラスの設定 (終了時の登録解除でも使う)
	WNDCLASS wc_{};

	///// ----- ===== ----- /////
};

