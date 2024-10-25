#include "WinApp.h"

void WinApp::Initialize()
{
	D3DResourceLeakChecker LeakCheak;




	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);

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


}

void WinApp::Update()
{
}
