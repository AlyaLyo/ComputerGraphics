#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

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