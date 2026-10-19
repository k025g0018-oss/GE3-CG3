#pragma once

#include <Windows.h>


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
	/// 更新
	/// </summary>
	void Update();

	///// ----- ===== ----- /////

private:
	///// ----- 関数 ----- /////


	///// ----- ===== ----- /////

	///// ----- 変数 ----- /////

	///// ----- ===== ----- /////
};

