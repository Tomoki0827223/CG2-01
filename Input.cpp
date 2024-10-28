#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dinput8.lib")

#include "Input.h"

void Input::Initialize(HINSTANCE hInstance, HWND hwnd)
{

	ComPtr<IDirectInput8> directinput = nullptr;
	result = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directinput, nullptr);
	assert(SUCCEEDED(result));

	ComPtr<IDirectInputDevice8> keybord;
	result = directinput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
	assert(SUCCEEDED(result));

	result = keyboard->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(result));

	result = keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));
}

void Input::Update()
{
}
