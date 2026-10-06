#pragma once

#include <windows.h>
#include <wrl.h>
#include <iostream>
#include <d3d.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <chrono>

#include "GameComponent.h"
#include "DisplayWin32.h"
#include "InputDevice.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

class Game 
{

public:

	Game(LPCWSTR applicationName, int screenHeight, int screenWidth);
	~Game();

	// fields

	ID3D11Texture2D* backBuffer;
	ID3D11RenderTargetView* renderView;

	//debugAnnotation;
	//renderSRV;

	Microsoft::WRL::ComPtr<ID3D11Device> device;
	ID3D11DeviceContext* context;
	IDXGISwapChain* swapChain;

	HINSTANCE instance;
	LPCWSTR name;

	bool IsScreenResized;

	std::chrono::time_point<std::chrono::steady_clock> prevTime;
	std::chrono::time_point<std::chrono::steady_clock> startTime;
	float totalTime;

	std::vector<GameComponent*> components;
	DisplayWin32* display;
	InputDevice* inputDevice;

	// methods

	void exit();
	void messageHandler();
	void restoreTargets();
	void run();

protected:

	virtual void destroyResources();
	virtual void draw();
	virtual void initialize();
	virtual void update();
	void endFrame();
	void prepareFrame();
	void prepareResources();
	void updateInternal();

private:

	bool isExitRequested;
	void createBackBuffer();

};

