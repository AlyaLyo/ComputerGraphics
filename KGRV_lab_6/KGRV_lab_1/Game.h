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
#include "Camera.h"
#include "PerSceneCb.h"
#include "DirectionalLight.h"

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

    ID3D11Texture2D* depthStencilBuffer;
    ID3D11DepthStencilView* depthStencilView;
    ID3D11DepthStencilState* depthStencilState;

    Microsoft::WRL::ComPtr<ID3D11Device> device;
    ID3D11DeviceContext* context;
    IDXGISwapChain* swapChain;

    HINSTANCE instance;
    LPCWSTR name;

    bool IsScreenResized;

    std::chrono::time_point<std::chrono::steady_clock> prevTime;
    std::chrono::time_point<std::chrono::steady_clock> startTime;

    float totalTime;
    float deltaTime;

    std::vector<GameComponent*> components;
    DisplayWin32* display = nullptr;
    InputDevice* inputDevice = nullptr;
    Camera* camera = nullptr;

    // CSM fields
    ID3D11Texture2D* shadowTexArr = nullptr;
    ID3D11DepthStencilView* depthShadowDsv = nullptr;
    ID3D11ShaderResourceView* depthShadowSrv = nullptr;
    ID3D11Buffer* perSceneCBuffer = nullptr;
    PerSceneCb sceneData;
    DirectionalLight dLight;

    // CSM getters
    ID3D11ShaderResourceView* GetCsm() const { return depthShadowSrv; }
    ID3D11Buffer* const* GetPerSceneCb() const { return &perSceneCBuffer; }
    DirectionalLight* GetDLight() { return &dLight; }

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
    void createCsmDepthTextureArray();

};