#include <Windows.h>
#include <cstdint>

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


int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

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

	OutputDebugStringA("Hallo,DirectX!\n");

	return 0;
}