#pragma once
#include <Windows.h>
#include <cstdint>
#include <wrl.h>


class WinApp
{
public:
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparm, LPARAM lparam);

public:

	void Initialize();
	void Update();
};

