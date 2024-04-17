#include <Windows.h>
#include <cstdint>
#include <string>
#include <format>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

#pragma region ツール
//ウインドウプローシャ
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {

	switch (msg)
	{

	case WM_DESTROY:

		PostQuitMessage(0);
		
		return 0;
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}

std::wstring ConvertString(const std::string& str) {
	if (str.empty()) {
		return std::wstring();
	}

	auto sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), NULL, 0);
	if (sizeNeeded == 0) {
		return std::wstring();
	}
	std::wstring result(sizeNeeded, 0);
	MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), &result[0], sizeNeeded);
	return result;
}

std::string ConvertString(const std::wstring& str) {
	if (str.empty()) {
		return std::string();
	}

	auto sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0, NULL, NULL);
	if (sizeNeeded == 0) {
		return std::string();
	}
	std::string result(sizeNeeded, 0);
	WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), sizeNeeded, NULL, NULL);
	return result;
}

void Log(const std::string& message) {

	OutputDebugStringA(message.c_str());

}

#pragma endregion


int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {


#pragma region Windowの生成

	WNDCLASS wc{};

	wc.lpfnWndProc = WindowProc;

	wc.lpszClassName = L"CG2WindowClass";

	wc.hInstance = GetModuleHandle(nullptr);

	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	RegisterClass(&wc);

	//クライアントの領域サイズ
	const int32_t kClientwidth = 1280;
	const int32_t kClientHeight = 720;

	RECT wrc = { 0,0,kClientwidth,kClientHeight };

	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	HWND hwnd = CreateWindow
	(
		//利用するクラス名
		wc.lpszClassName,
		//タイトルバーの文字（何でもいい）
		L"CG2",
		//よく見るウインドウスタイル
		WS_OVERLAPPEDWINDOW,
		//表示X座標（Windowsに任せる）
		CW_USEDEFAULT,
		//表示Y座標（WindowsOSにまかせる）
		CW_USEDEFAULT,
		//ウインドウ横幅
		wrc.right - wrc.left,
		//ウインドウ立幅
		wrc.bottom - wrc.top,
		//親ウインドウハンドル
		nullptr,
		//メニューハンドル
		nullptr,
		//インスタンスハンドル
		wc.hInstance,
		//オプションハンドル
		nullptr
	);

	//ウインドウを表示する
	ShowWindow(hwnd, SW_SHOW);

#pragma endregion

#pragma region DXGIFactryの生成
	//dxgiFactoryの生成
	IDXGIFactory7* dxgiFactory = nullptr;
	//HRESULT→Windows系のエラーコード
	//関数が成功したかどうかをSUCCEEDEDマクロで判定できる
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	IDXGIAdapter4* useadapter = nullptr;
	assert(SUCCEEDED(hr));
#pragma endregion


#pragma region 使用アダプタ(GPU)の決定

	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
		IID_PPV_ARGS(&useadapter)) != DXGI_ERROR_NOT_FOUND; i++)
	{
		//アダプター（GPU）の情報を取得する
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useadapter->GetDesc3(&adapterDesc);
		//取得できないのは一大事
		assert(SUCCEEDED(hr));

		//ソフトウエアでなければ採用！
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE))
		{
			Log(ConvertString(std::format(L"Use Adapater:{}\n", adapterDesc.Description)));

			break;
		}
		useadapter = nullptr;
	}
	assert(useadapter != nullptr);
#pragma endregion

#pragma region D3D12Deviceの生成

	ID3D12Device* device = nullptr;
	D3D_FEATURE_LEVEL featureLevels[] = {
		
		D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0
	
	};
	const char* featureLevelStrrings[] = { "12.2","12.1","12.0" };

	for (size_t i = 0; i < _countof(featureLevels); i++)
	{
		hr = D3D12CreateDevice(useadapter, featureLevels[i], IID_PPV_ARGS(&device));

		if (SUCCEEDED(hr))
		{
			Log(std::format("FeatureLevel : {}\n", featureLevelStrrings[i]));
			break;
		}
	}
	//デバイスの生成がうまくいかなかったので起動できない
	assert(device != nullptr);
	Log("Complete create D3D12Device!!!\n");

#pragma endregion


	MSG msg{};

	//Windowsの罰ボタンが押されるまでループ
	while (msg.message != WM_QUIT)
	{

		//Windowsにメッセージが来てたら最優先で処理させる
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			//ゲームの処理
		}
	}

	Log("Hallo,DirectX!\n");

	return 0;
}