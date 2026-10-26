#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <ostream>
#include <string>

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

	///// ----- getter ----- /////

	/// <summary>
	/// デバイス
	/// </summary>
	/// <returns></returns>
	ID3D12Device* GetDevice() const {
		return device_.Get();
	}

	IDXGIFactory7* GetDxgiFactory() const {
		return dxgiFactory_.Get();
	}

	const std::string& GetAdapterName() const {
		return adapterName_;
	}

	///// ----- ===== ----- /////

private:
	///// ----- メンバ関数 ----- /////

	/// <summary>
	/// デバイスの初期化
	/// </summary>
	/// <param name="logStream"></param>
	void InitializeDevice(std::ostream& logStream);

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

	///// ----- ===== ----- /////
};

