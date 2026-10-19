#include "Window.h"
#include "externals/imgui/imgui.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#include <cstdint>

/// <summary>
/// ウィンドウプロシージャ
/// </summary>
/// <param name="hwnd"></param>
/// <param name="msg"></param>
/// <param name="wparam"></param>
/// <param name="lparam"></param>
/// <returns></returns>
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	// ImGui
#if USE_IMGUI
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
		return true;
	}
#endif // USE_IMGUI

	/// --- ウィンドウの比率 (16:9) ---

	// ドラッグしても比率が変わらないようにする
	const LONG kAspectWidth = 16;
	const LONG kAspectHeight = 9;

	// メッセージに応じてゲーム固有の処理を行う
	switch (msg) {
		// ウィンドウが破棄された
		case WM_DESTROY:
			// OSに対して、アプリの終了を伝える
			PostQuitMessage(0);
			return 0;

			// ウィンドウの端をドラッグして大きさを変えている途中
		case WM_SIZING:
		{
			// これから変わる予定のウィンドウの大きさ
			RECT* rect = reinterpret_cast<RECT*>(lparam);

			// 枠とタイトルバーの文の大きさを求める
			RECT frame = {0, 0, 0, 0};
			AdjustWindowRect(&frame, WS_OVERLAPPEDWINDOW, FALSE);
			const LONG frameWidth = frame.right - frame.left;
			const LONG frameHeight = frame.bottom - frame.top;

			// 中身(描画部分の大きさ)
			LONG clientWidth = (rect->right - rect->left) - frameWidth;
			LONG clientHeight = (rect->bottom - rect->top) - frameHeight;

			switch (wparam) {
				// 上か下をつかんだときは、高さに合わせて幅を決める
				case WMSZ_TOP:
				case WMSZ_BOTTOM:
					clientWidth = clientHeight * kAspectWidth / kAspectHeight;
					rect->right = rect->left + clientWidth + frameWidth;
					break;

					//上側の角をつかんだときは、幅に合わせて上の辺を動かす
				case WMSZ_TOPLEFT:
				case WMSZ_TOPRIGHT:
					clientHeight = clientWidth * kAspectHeight / kAspectWidth;
					rect->top = rect->bottom - clientHeight - frameHeight;
					break;

					// 左右・下側の角をつかんだときは、幅に合わせて下の辺を動かす
				default:
					clientHeight = clientWidth * kAspectHeight / kAspectWidth;
					rect->bottom = rect->top + clientHeight + frameHeight;
					break;
			}

			return TRUE;
		}
	}

	// 標準のメッセージ処理を行う
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

/// <summary>
/// 初期化
/// </summary>
void Window::Initialize() {

	///// ----- ウィンドウ ----- /////

	// COMの初期化 このプログラムでCOMという仕組みを使う
	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);

	WNDCLASS wc{};
	// ウィンドウプロシージャ
	wc.lpfnWndProc = WindowProc;
	// ウィンドウクラス名(何でもよい)
	wc.lpszClassName = L"CG2WindowClass";
	// インスタンスハンドル
	wc.hInstance = GetModuleHandle(nullptr);
	// カーソル
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	// ウィンドウクラスを登録する
	RegisterClass(&wc);

	// クライアント領域のサイズ (ゲーム内部の解像度)
	const int32_t kClientWidth = 1280;
	const int32_t kClientHeight = 720;

	// ウィンドウサイズを表す構造体にクライアント領域を入れる
	RECT wrc = {0, 0, kClientWidth, kClientHeight};
	// クライアント領域をもとに実際のサイズにwrcを変更してもらう
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	// ウィンドウの生成
	HWND hwnd = CreateWindow(
		wc.lpszClassName,	  // 利用するクラス名
		L"CG2",				  // タイトルバーの文字
		WS_OVERLAPPEDWINDOW,  // ウィンドウスタイル
		CW_USEDEFAULT,		  // 表示X座標(Windowsに任せる)
		CW_USEDEFAULT,		  // 表示Y座標(windowsOSに任せる)
		wrc.right - wrc.left, // ウィンドウ横幅
		wrc.bottom - wrc.top, // ウィンドウ縦幅
		nullptr,			  // 親ウィンドウハンドル
		nullptr,			  // メニューハンドル
		wc.hInstance,		  // インスタンスハンドル
		nullptr				  // オプション
	);

	/// --- ウィンドウサイズ ---

	// クライアント領域のサイズ
	const int kWindowWidth = 1600;
	const int kWindowHeight = 900;

	// 中身(描画部分)がこの大きさになるように、枠とタイトルバーの分を足す
	RECT windowRect = {0, 0, kWindowWidth, kWindowHeight};
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	// ウィンドウの大きさを変える (位置はそのまま)
	SetWindowPos(hwnd, nullptr, 0, 0, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, SWP_NOMOVE | SWP_NOZORDER);

	// ウィンドウを表示する
	ShowWindow(hwnd, SW_SHOW);

	// クライアント領域をもとに実際のサイズにwrcを変更してもらう
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	// ウィンドウを表示する
	ShowWindow(hwnd, SW_SHOW);

	///// ----- ===== ----- /////
}

/// <summary>
/// 更新
/// </summary>
void Window::Update() {

}