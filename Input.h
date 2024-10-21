#pragma once

#include <Windows.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dinput8.lib")

#include <cassert>
#include <wrl.h>

class Input
{
public:

	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	void Initialize(HINSTANCE hInstance, HWND hwnd);

	void Update();

	bool PushKey(BYTE keyNumber);

	bool TriggerKey(BYTE keyNumber);

private:

	BYTE keyPre[256] = {};
	BYTE key[256] = {};

	ComPtr<IDirectInputDevice8> keyboard;

	IDirectInput8* directInput = nullptr;

	HRESULT result;


};

