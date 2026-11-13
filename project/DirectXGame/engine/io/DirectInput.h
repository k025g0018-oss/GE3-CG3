#pragma once

#include "Window.h"
#include "Vector.h"

#include <wrl.h>
#include <cstdint>

#define DIRECTINPUT_VERSION 0x0800 // DirectInputのバージョン指定
#include <dinput.h>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

// 前方宣言
class Window;

///// ----- DirectInput ----- /////

class DirectInput {
public:
	DirectInput();
	~DirectInput();

	/// <summary>
	/// 初期化処理
	/// </summary>
	// 入力の生成に使うWindowを受け取る
	void Initialize(Window* window);

	/// <summary>
	/// 更新処理
	/// </summary>
	// 毎フレームのキーボード状態を取得する
	void Update();

	/// <summary>
	/// 描画処理
	/// </summary>

	/// <summary>
	/// 解放処理
	/// </summary>
	// キーボード入力取得をやめて、デバイスとDirectInput本体を開放する
	void Finalize();

	/// <summary>
	/// キーが押されている間trueを返す
	/// </summary>
	bool IsPress(uint8_t keyNumber) const;

	/// <summary>
	/// キーを押した瞬間だけtrueを返す
	/// </summary>
	bool IsTrigger(uint8_t keyNumber) const;

	/// <summary>
	/// マウスのボタンが押されている間trueを返す
	/// </summary>
	/// <param name="buttonNumber">0:左、1:右、2:中</param>
	bool IsMousePress(uint32_t buttonNumber) const;

	/// <summary>
	/// マウスのボタンを押した瞬間だけtrueを返す
	/// </summary>
	/// <param name="buttonNumber">0:左、1:右、2:中</param>
	bool IsMouseTrigger(uint32_t buttonNumber) const;

	/// <summary>
	/// マウスカーソルの位置を返す (ウィンドウのクライアント領域の左上が(0,0)、ピクセル)
	/// </summary>
	const Vector2& GetMousePosition() const {
		return mousePosition_;
	}

private:
	///// ----- 関数 ----- /////


	///// ----- ===== ----- /////

	///// ----- メンバ変数 ----- /////

	// WindowsAPI
	Window* window_ = nullptr;

	// DirectInput本体を自動解放する
	Microsoft::WRL::ComPtr<IDirectInput8> directInput_;

	// 毎フレーム入力状態を取得するキーボードデバイス
	Microsoft::WRL::ComPtr<IDirectInputDevice8> keyboard_;

	// 毎フレーム入力状態を取得するマウスデバイス
	Microsoft::WRL::ComPtr<IDirectInputDevice8> mouse_;

	// 現在のフレームのキー入力状態
	BYTE keys_[256]{};

	// 1フレーム前のキー入力状態
	BYTE previousKeys_[256]{};

	// 現在のフレームのマウス入力状態 (ボタンと移動量)
	DIMOUSESTATE2 mouseState_{};

	// 1フレーム前のマウス入力状態
	DIMOUSESTATE2 previousMouseState_{};

	// マウスカーソルの位置 (クライアント座標、ピクセル)
	Vector2 mousePosition_{0.0f, 0.0f};

	///// ----- ===== ----- /////
};