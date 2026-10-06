#include "SkyboxComponent.h"
#include "Game.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

SkyboxComponent::SkyboxComponent(Game* game, std::wstring cubemapPath)
    : GameComponent(game),
    cubemapPath(cubemapPath)
{
    generateCubeGeometry();
    game->components.insert(game->components.begin(), this);
}

SkyboxComponent::~SkyboxComponent()
{
}

void SkyboxComponent::initialize()
{
    // Compile vertex shader
    ID3DBlob* errorVertexCode = nullptr;
    auto res = D3DCompileFromFile(L"./Shaders/SkyboxShader.hlsl",
        nullptr, nullptr,
        "VSMain", "vs_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        &vertexShaderByteCode,
        &errorVertexCode);

    if (FAILED(res)) {
        if (errorVertexCode) {
            char* compileErrors = (char*)(errorVertexCode->GetBufferPointer());
            std::cout << compileErrors << std::endl;
        }
        else {
            MessageBox(game->display->hWnd, L"SkyboxShader.hlsl", L"Missing Shader File", MB_OK);
        }
        return;
    }

    // Compile pixel shader
    ID3DBlob* errorPixelCode = nullptr;
    res = D3DCompileFromFile(L"./Shaders/SkyboxShader.hlsl",
        nullptr, nullptr,
        "PSMain", "ps_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        &pixelShaderByteCode,
        &errorPixelCode);

    if (FAILED(res)) {
        if (errorPixelCode) {
            char* compileErrors = (char*)(errorPixelCode->GetBufferPointer());
            std::cout << compileErrors << std::endl;
        }
        return;
    }

    game->device->CreateVertexShader(
        vertexShaderByteCode->GetBufferPointer(),
        vertexShaderByteCode->GetBufferSize(),
        nullptr, &vertexShader);

    game->device->CreatePixelShader(
        pixelShaderByteCode->GetBufferPointer(),
        pixelShaderByteCode->GetBufferSize(),
        nullptr, &pixelShader);

    D3D11_INPUT_ELEMENT_DESC inputElements[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    game->device->CreateInputLayout(
        inputElements, 1,
        vertexShaderByteCode->GetBufferPointer(),
        vertexShaderByteCode->GetBufferSize(),
        &layout);


    HRESULT hr = CreateDDSTextureFromFile(
        game->device.Get(),
        cubemapPath.c_str(),
        nullptr,
        &cubemapSRV);

    if (FAILED(hr))
    {
        std::wcerr << L"Failed to load skybox cubemap: " << cubemapPath
            << L" HRESULT: 0x" << std::hex << hr << std::endl;
    }
    else
    {
        std::wcout << L"Skybox cubemap loaded: " << cubemapPath << std::endl;
    }

    D3D11_BUFFER_DESC vertexBufDesc = {};
    vertexBufDesc.Usage = D3D11_USAGE_IMMUTABLE;
    vertexBufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufDesc.ByteWidth = sizeof(Point) * static_cast<UINT>(points_.size());

    D3D11_SUBRESOURCE_DATA vertexData = {};
    vertexData.pSysMem = points_.data();

    game->device->CreateBuffer(&vertexBufDesc, &vertexData, &vb);


    D3D11_BUFFER_DESC indexBufDesc = {};
    indexBufDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufDesc.ByteWidth = sizeof(int) * static_cast<UINT>(indeces_.size());

    D3D11_SUBRESOURCE_DATA indexData = {};
    indexData.pSysMem = indeces_.data();

    game->device->CreateBuffer(&indexBufDesc, &indexData, &ib);

    strides[0] = sizeof(Point);  // 16 bytes (Vector4)
    offsets[0] = 0;

    D3D11_BUFFER_DESC constBufDesc = {};
    constBufDesc.Usage = D3D11_USAGE_DEFAULT;
    constBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constBufDesc.ByteWidth = sizeof(CBPerObject);

    game->device->CreateBuffer(&constBufDesc, nullptr, &cb);

    CD3D11_RASTERIZER_DESC rastDesc = {};
    rastDesc.CullMode = D3D11_CULL_NONE;
    rastDesc.FillMode = D3D11_FILL_SOLID;
    rastDesc.FrontCounterClockwise = false;
    rastDesc.DepthClipEnable = true;

    game->device->CreateRasterizerState(&rastDesc, &rastState);

    D3D11_DEPTH_STENCIL_DESC depthDesc = {};
    depthDesc.DepthEnable = true;
    depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    depthDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    depthDesc.StencilEnable = false;

    game->device->CreateDepthStencilState(&depthDesc, &depthState);

    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    game->device->CreateSamplerState(&samplerDesc, &samplerState);
}

void SkyboxComponent::draw()
{

    ID3D11DepthStencilState* prevDepthState = nullptr;
    UINT prevStencilRef = 0;
    game->context->OMGetDepthStencilState(&prevDepthState, &prevStencilRef);

    game->context->OMSetDepthStencilState(depthState, 0);

    game->context->RSSetState(rastState);

    D3D11_VIEWPORT viewport = {};
    viewport.Width = static_cast<float>(game->display->clientWidth);
    viewport.Height = static_cast<float>(game->display->clientHeight);
    viewport.MinDepth = 0;
    viewport.MaxDepth = 1.0f;
    game->context->RSSetViewports(1, &viewport);

    game->context->IASetInputLayout(layout);
    game->context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    game->context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
    game->context->IASetVertexBuffers(0, 1, &vb, strides, offsets);

    game->context->VSSetShader(vertexShader, nullptr, 0);
    game->context->VSSetConstantBuffers(0, 1, &cb);

    game->context->PSSetShader(pixelShader, nullptr, 0);
    game->context->PSSetShaderResources(0, 1, &cubemapSRV);
    game->context->PSSetSamplers(0, 1, &samplerState);

    game->context->DrawIndexed(static_cast<UINT>(indeces_.size()), 0, 0);

    ID3D11ShaderResourceView* nullSrv = nullptr;
    game->context->PSSetShaderResources(0, 1, &nullSrv);

    game->context->OMSetDepthStencilState(prevDepthState, prevStencilRef);
    if (prevDepthState) prevDepthState->Release();
}

void SkyboxComponent::destroyResources()
{
}

void SkyboxComponent::update()
{

    Matrix world = Matrix::CreateTranslation(game->camera->GetPosition());


    Matrix viewNoTranslation = game->camera->GetView();
    viewNoTranslation._41 = 0.0f;
    viewNoTranslation._42 = 0.0f;
    viewNoTranslation._43 = 0.0f;

    CBPerObject cbData;
    cbData.worldViewProj = (viewNoTranslation * game->camera->GetProj()).Transpose();
    cbData.world = Matrix::Identity.Transpose();
    game->context->UpdateSubresource(cb, 0, nullptr, &cbData, 0, 0);
}

void SkyboxComponent::reload()
{
}

void SkyboxComponent::generateCubeGeometry()
{

    float s = 1.0f;
    points_ = {
        // Front face (+Z)
        { Vector4(-s, -s,  s, 1.0f) },
        { Vector4(s, -s,  s, 1.0f) },
        { Vector4(s,  s,  s, 1.0f) },
        { Vector4(-s,  s,  s, 1.0f) },
        // Back face (-Z)
        { Vector4(s, -s, -s, 1.0f) },
        { Vector4(-s, -s, -s, 1.0f) },
        { Vector4(-s,  s, -s, 1.0f) },
        { Vector4(s,  s, -s, 1.0f) },
        // Left face (-X)
        { Vector4(-s, -s, -s, 1.0f) },
        { Vector4(-s, -s,  s, 1.0f) },
        { Vector4(-s,  s,  s, 1.0f) },
        { Vector4(-s,  s, -s, 1.0f) },
        // Right face (+X)
        { Vector4(s, -s,  s, 1.0f) },
        { Vector4(s, -s, -s, 1.0f) },
        { Vector4(s,  s, -s, 1.0f) },
        { Vector4(s,  s,  s, 1.0f) },
        // Top face (+Y)
        { Vector4(-s,  s,  s, 1.0f) },
        { Vector4(s,  s,  s, 1.0f) },
        { Vector4(s,  s, -s, 1.0f) },
        { Vector4(-s,  s, -s, 1.0f) },
        // Bottom face (-Y)
        { Vector4(-s, -s, -s, 1.0f) },
        { Vector4(s, -s, -s, 1.0f) },
        { Vector4(s, -s,  s, 1.0f) },
        { Vector4(-s, -s,  s, 1.0f) },
    };

    indeces_ = {
        // Front
        0,  1,  2,   0,  2,  3,
        // Back
        4,  5,  6,   4,  6,  7,
        // Left
        8,  9, 10,   8, 10, 11,
        // Right
        12, 13, 14,  12, 14, 15,
        // Top
        16, 17, 18,  16, 18, 19,
        // Bottom
        20, 21, 22,  20, 22, 23,
    };
}