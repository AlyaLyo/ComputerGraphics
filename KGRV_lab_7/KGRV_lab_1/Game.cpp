#include "Game.h"
#include "InputDevice.h"

using namespace DirectX;
using namespace SimpleMath;

Game::Game(LPCWSTR applicationName, int screenHeight, int screenWidth)
    : dLight(this)
{
    instance = GetModuleHandle(nullptr);
    display = new DisplayWin32(instance, applicationName, screenWidth, screenHeight);
    inputDevice = new InputDevice(this);
    camera = new Camera(this);
    camera->aspectRatio = static_cast<float>(screenWidth) / static_cast<float>(screenHeight);
    D3D_FEATURE_LEVEL featureLevel[] = { D3D_FEATURE_LEVEL_11_1 };

    DXGI_SWAP_CHAIN_DESC swapDesc = {};
    swapDesc.BufferCount = 2;
    swapDesc.BufferDesc.Width = screenWidth;
    swapDesc.BufferDesc.Height = screenHeight;
    swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapDesc.OutputWindow = display->hWnd;
    swapDesc.Windowed = true;
    swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    swapDesc.SampleDesc.Count = 1;
    swapDesc.SampleDesc.Quality = 0;

    auto res = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        D3D11_CREATE_DEVICE_DEBUG,
        featureLevel,
        1,
        D3D11_SDK_VERSION,
        &swapDesc,
        &swapChain,
        &device,
        nullptr,
        &context);

    if (FAILED(res))
    {
        // Well, that was unexpected
        std::wstring errorMsg = L"Failed to create DirectX device. Error code: " + std::to_wstring(res);
        MessageBox(nullptr, errorMsg.c_str(), L"DirectX Error", MB_ICONERROR | MB_OK);
    }

    createBackBuffer();

    D3D11_TEXTURE2D_DESC depthTexDesc = {};
    depthTexDesc.Width = display->clientWidth;
    depthTexDesc.Height = display->clientHeight;
    depthTexDesc.MipLevels = 1;
    depthTexDesc.ArraySize = 1;
    depthTexDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthTexDesc.SampleDesc.Count = 1;
    depthTexDesc.SampleDesc.Quality = 0;
    depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
    depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthTexDesc.CPUAccessFlags = 0;
    depthTexDesc.MiscFlags = 0;

    res = device->CreateTexture2D(&depthTexDesc, nullptr, &depthStencilBuffer);
    if (FAILED(res))
    {
        std::wstring errorMsg = L"Failed to create depth stencil buffer. Error code: " + std::to_wstring(res);
        MessageBox(nullptr, errorMsg.c_str(), L"DirectX Error", MB_ICONERROR | MB_OK);
    }

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;

    res = device->CreateDepthStencilView(depthStencilBuffer, &dsvDesc, &depthStencilView);
    if (FAILED(res))
    {
        std::wstring errorMsg = L"Failed to create depth stencil view. Error code: " + std::to_wstring(res);
        MessageBox(nullptr, errorMsg.c_str(), L"DirectX Error", MB_ICONERROR | MB_OK);
    }

    D3D11_DEPTH_STENCIL_DESC dsDesc = {};
    dsDesc.DepthEnable = true;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
    dsDesc.StencilEnable = false;

    res = device->CreateDepthStencilState(&dsDesc, &depthStencilState);
    if (FAILED(res))
    {
        std::wstring errorMsg = L"Failed to create depth stencil state. Error code: " + std::to_wstring(res);
        MessageBox(nullptr, errorMsg.c_str(), L"DirectX Error", MB_ICONERROR | MB_OK);
    }

    // Create CSM depth texture array
    createCsmDepthTextureArray();

    // Create per-scene constant buffer
    D3D11_BUFFER_DESC constBufPerSceneDesc = {};
    constBufPerSceneDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constBufPerSceneDesc.Usage = D3D11_USAGE_DEFAULT;
    constBufPerSceneDesc.CPUAccessFlags = 0;
    constBufPerSceneDesc.MiscFlags = 0;
    constBufPerSceneDesc.StructureByteStride = 0;
    constBufPerSceneDesc.ByteWidth = sizeof(PerSceneCb);

    device->CreateBuffer(&constBufPerSceneDesc, nullptr, &perSceneCBuffer);
}

Game::~Game()
{
    for (auto component : components)
    {
        delete component;
    }
    delete display;
    delete inputDevice;
    delete camera;
    if (perSceneCBuffer) perSceneCBuffer->Release();
    if (depthShadowSrv) depthShadowSrv->Release();
    if (depthShadowDsv) depthShadowDsv->Release();
    if (shadowTexArr) shadowTexArr->Release();
    if (depthStencilState) depthStencilState->Release();
    if (depthStencilView) depthStencilView->Release();
    if (depthStencilBuffer) depthStencilBuffer->Release();
    context->Release();
    backBuffer->Release();
    renderView->Release();
    swapChain->Release();
}

void Game::createCsmDepthTextureArray()
{
    D3D11_TEXTURE2D_DESC depthDescription = {};
    depthDescription.Width = 1024;
    depthDescription.Height = 1024;
    depthDescription.ArraySize = 5; // CASCADE_COUNT + 1
    depthDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
    depthDescription.Format = DXGI_FORMAT_R32_TYPELESS;
    depthDescription.MipLevels = 1;
    depthDescription.SampleDesc.Count = 1;
    depthDescription.SampleDesc.Quality = 0;
    depthDescription.Usage = D3D11_USAGE_DEFAULT;
    depthDescription.CPUAccessFlags = 0;
    depthDescription.MiscFlags = 0;

    auto res = device->CreateTexture2D(&depthDescription, nullptr, &shadowTexArr);
    if (FAILED(res))
    {
        OutputDebugString(TEXT("Fatal error: Failed to create CSM depth texture array!\n"));
    }

    D3D11_DEPTH_STENCIL_VIEW_DESC dViewDesc = {};
    dViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
    dViewDesc.Texture2DArray.MipSlice = 0;
    dViewDesc.Texture2DArray.FirstArraySlice = 0;
    dViewDesc.Texture2DArray.ArraySize = 5;

    res = device->CreateDepthStencilView(shadowTexArr, &dViewDesc, &depthShadowDsv);
    if (FAILED(res))
    {
        OutputDebugString(TEXT("Fatal error: Failed to create CSM depth stencil view!\n"));
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    srvDesc.Texture2DArray.MostDetailedMip = 0;
    srvDesc.Texture2DArray.MipLevels = 1;
    srvDesc.Texture2DArray.FirstArraySlice = 0;
    srvDesc.Texture2DArray.ArraySize = 5;

    res = device->CreateShaderResourceView(shadowTexArr, &srvDesc, &depthShadowSrv);
    if (FAILED(res))
    {
        OutputDebugString(TEXT("Fatal error: Failed to create CSM depth SRV!\n"));
    }
}

void Game::createBackBuffer()
{
    auto res = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
    res = device->CreateRenderTargetView(backBuffer, nullptr, &renderView);
}

void Game::exit()
{
    destroyResources();
}

void Game::initialize()
{
    for (auto component : components)
    {
        component->initialize();
    }
}

void Game::draw()
{
    float color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    context->ClearRenderTargetView(renderView, color);
    context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    for (auto component : components)
    {
        component->draw();
    }
}

void Game::prepareFrame()
{
    context->ClearState();

    context->OMSetRenderTargets(0, nullptr, depthShadowDsv);

    context->ClearDepthStencilView(depthShadowDsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    for (auto component : components)
    {
        component->prepareFrame();
    }
}

void Game::destroyResources()
{
    for (auto component : components)
    {
        component->destroyResources();
    }
}

void Game::update()
{
    auto tmp = Vector4(1.0f, 1.0f, 1.0f, 0.0f);
    tmp.Normalize();
    dLight.SetDirection(tmp);

    sceneData.LightPos = dLight.GetDirection();
    sceneData.LightPos.Normalize();
    auto camPos = camera->GetPosition();
    sceneData.CameraPos = Vector4(camPos.x, camPos.y, camPos.z, 1.0f);
    sceneData.LightColor = dLight.GetColor();
    sceneData.AmbientSpecularPowType = Vector4(0.15f, 0.5f, 32.0f, 0.0f);

    // visualisation
    sceneData.AmbientSpecularPowType = Vector4(0.15f, 0.5f, 32.0f, 0.0f);

    sceneData.T = Matrix(
        0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, -0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 1.0f).Transpose();
    context->UpdateSubresource(perSceneCBuffer, 0, nullptr, &sceneData, 0, 0);

    if (camera && inputDevice)
    {
        camera->onMouseMove();
    }
    if (camera)
    {
        camera->update();
    }
    for (auto component : components)
    {
        component->update();
    }
}

void Game::run()
{
    initialize();
    prevTime = std::chrono::steady_clock::now();
    totalTime = 0;
    unsigned int frameCount = 0;

    isExitRequested = false;
    while (!isExitRequested)
    {
        messageHandler();

        auto    curTime = std::chrono::steady_clock::now();
        deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(curTime - prevTime).count() / 1000000.0f;
        prevTime = curTime;

        totalTime += deltaTime;
        frameCount++;

        if (totalTime > 1.0f)
        {
            float fps = frameCount / totalTime;

            totalTime -= 1.0f;

            WCHAR text[256];
            swprintf_s(text, TEXT("FPS: %f"), fps);
            SetWindowText(display->hWnd, text);

            frameCount = 0;
        }

        update();
        prepareFrame();

        context->ClearState();
        context->OMSetDepthStencilState(depthStencilState, 1);
        context->OMSetRenderTargets(1, &renderView, depthStencilView);

        draw();

        context->OMSetRenderTargets(0, nullptr, nullptr);
        swapChain->Present(1, 0);
    }
    exit();
}

void Game::messageHandler()
{
    MSG msg = {};

    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        if (msg.message == WM_INPUT)
        {
            UINT dwSize = 0;
            GetRawInputData(reinterpret_cast<HRAWINPUT>(msg.lParam), RID_INPUT, nullptr, &dwSize, sizeof(RAWINPUTHEADER));
            LPBYTE lpb = new BYTE[dwSize];
            if (lpb == nullptr)
            {
                return;
            }

            if (GetRawInputData((HRAWINPUT)msg.lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
                OutputDebugString(TEXT("GetRawInputData does not return correct size !\n"));

            RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(lpb);

            if (raw->header.dwType == RIM_TYPEKEYBOARD)
            {
                inputDevice->OnKeyDown({
                        raw->data.keyboard.MakeCode,
                        raw->data.keyboard.Flags,
                        raw->data.keyboard.VKey,
                        raw->data.keyboard.Message
                    });
            }
            else if (raw->header.dwType == RIM_TYPEMOUSE)
            {
                inputDevice->OnMouseMove({
                        raw->data.mouse.usFlags,
                        raw->data.mouse.usButtonFlags,
                        static_cast<int>(raw->data.mouse.ulExtraInformation),
                        static_cast<int>(raw->data.mouse.ulRawButtons),
                        static_cast<short>(raw->data.mouse.usButtonData),
                        raw->data.mouse.lLastX,
                        raw->data.mouse.lLastY
                    });
            }
            delete[] lpb;
        }
    }

    if (msg.message == WM_QUIT)
    {
        isExitRequested = true;
        return;
    }
}

void Game::restoreTargets()
{
}

void Game::prepareResources()
{
}

void Game::endFrame()
{
}

void Game::updateInternal()
{
}
