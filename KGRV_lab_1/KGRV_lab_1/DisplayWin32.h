#pragma once

#include <WinUser.h>
#include <windows.h>
#include <iostream>

class DisplayWin32
{

public:

	DisplayWin32(HINSTANCE instance, LPCWSTR applicationName, int screenHeight, int screenWidth);

	int clientHeight;
	int clientWidth;

	HINSTANCE hInstance;
	HWND hWnd;
	HMODULE module;

	WNDCLASSEX wc;

};