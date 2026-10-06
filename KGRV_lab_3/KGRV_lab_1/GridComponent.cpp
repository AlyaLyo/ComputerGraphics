#include "Game.h"
#include "GridComponent.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

GridComponent::GridComponent(Game* game, float size, int divisionsAmount) :
    GameComponent(game),
    size(size),
    divisionsAmount(divisionsAmount)
{
    reset();
    game->components.push_back(this);
}

GridComponent::~GridComponent()
{
}

void GridComponent::initialize()
{
    ID3DBlob* errorVertexCode = nullptr;
    auto res = D3DCompileFromFile(L"./Shaders/MyVeryFirstShader.hlsl",
        nullptr /*macros*/,
        nullptr /*include*/,
        "VSMain",
        "vs_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        &vertexShaderByteCode,
        &errorVertexCode);

    if (FAILED(res)) {
        if (errorVertexCode) {
            char* compileErrors = (char*)(errorVertexCode->GetBufferPointer());

            std::cout << compileErrors << std::endl;
        }
        else
        {
            MessageBox(game->display->hWnd, L"MyVeryFirstShader.hlsl", L"Missing Shader File", MB_OK);
        }
        return;
    }

    D3D_SHADER_MACRO Shader_Macros[] = { "TEST", "1", "TCOLOR", "float4(0.0f, 1.0f, 0.0f, 1.0f)", nullptr, nullptr };

    ID3DBlob* errorPixelCode;
    res = D3DCompileFromFile(L"./Shaders/MyVeryFirstShader.hlsl",
        Shader_Macros /*macros*/,
        nullptr /*include*/, 
        "PSMain",
        "ps_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        &pixelShaderByteCode,
        &errorPixelCode);

    game->device->CreateVertexShader(
        vertexShaderByteCode->GetBufferPointer(),
        vertexShaderByteCode->GetBufferSize(),
        nullptr, &vertexShader);

    game->device->CreatePixelShader(
        pixelShaderByteCode->GetBufferPointer(),
        pixelShaderByteCode->GetBufferSize(),
        nullptr, &pixelShader);

    D3D11_INPUT_ELEMENT_DESC inputElements[] = {
    D3D11_INPUT_ELEMENT_DESC {
            "POSITION",
            0,
            DXGI_FORMAT_R32G32B32A32_FLOAT,
            0,
            0,
            D3D11_INPUT_PER_VERTEX_DATA,
            0},
    D3D11_INPUT_ELEMENT_DESC {
            "COLOR",
            0,
            DXGI_FORMAT_R32G32B32A32_FLOAT,
            0,
            D3D11_APPEND_ALIGNED_ELEMENT,
            D3D11_INPUT_PER_VERTEX_DATA,
            0}
    };

    game->device->CreateInputLayout(
        inputElements,
        2,
        vertexShaderByteCode->GetBufferPointer(),
        vertexShaderByteCode->GetBufferSize(),
        &layout);

    D3D11_BUFFER_DESC vertexBufDesc = {};
    vertexBufDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexBufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vertexBufDesc.MiscFlags = 0;
    vertexBufDesc.StructureByteStride = 0;
    vertexBufDesc.ByteWidth = sizeof(Vector4) * std::size(points_) * 2;

    D3D11_SUBRESOURCE_DATA vertexData = {};
    vertexData.pSysMem = points_.data();
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    game->device->CreateBuffer(&vertexBufDesc, &vertexData, &vb);

    strides[0] = 32;
    offsets[0] = 0;

    D3D11_BUFFER_DESC constBufDesc = {};
    constBufDesc.Usage = D3D11_USAGE_DEFAULT;
    constBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constBufDesc.CPUAccessFlags = 0;
    constBufDesc.MiscFlags = 0;
    constBufDesc.StructureByteStride = 0;
    constBufDesc.ByteWidth = sizeof(SimpleMath::Matrix);

    game->device->CreateBuffer(&constBufDesc, nullptr, &cb);

    CD3D11_RASTERIZER_DESC rastDesc = {};
    rastDesc.CullMode = D3D11_CULL_NONE;
    rastDesc.FillMode = D3D11_FILL_SOLID;
    rastDesc.FrontCounterClockwise = false;
    rastDesc.DepthClipEnable = true;

    res = game->device->CreateRasterizerState(&rastDesc, &rastState);
}

void GridComponent::draw()
{
    game->context->RSSetState(rastState);

    D3D11_VIEWPORT viewport = {};
    viewport.Width = static_cast<float>(game->display->clientWidth);
    viewport.Height = static_cast<float>(game->display->clientHeight);
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.MinDepth = 0;
    viewport.MaxDepth = 1.0f;

    game->context->RSSetViewports(1, &viewport);

    game->context->IASetInputLayout(layout);
    game->context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
    game->context->IASetVertexBuffers(0, 1, &vb, strides, offsets);
    game->context->VSSetShader(vertexShader, nullptr, 0);
    game->context->VSSetConstantBuffers(0, 1, &cb);
    game->context->PSSetShader(pixelShader, nullptr, 0);
    game->context->Draw(points_.size(), 0);
}

void GridComponent::destroyResources()
{
}

void GridComponent::update()
{
    Matrix worldViewProj = game->camera->getMatrix();
    worldViewProj = worldViewProj.Transpose();
    game->context->UpdateSubresource(cb, 0, nullptr, &worldViewProj, 0, 0);
}

void GridComponent::reload()
{
}

void GridComponent::reset()
{
    float half = size * 0.5f;
    float step = size / divisionsAmount;

    constexpr Vector4 regularColor(0.5f, 0.5f, 0.5f, 1.0f);
    constexpr Vector4 majorColor(0.9f, 0.9f, 0.9f, 1.0f);

    for (int i = 0; i <= divisionsAmount; ++i)
    {
        float p = -half + i * step;
        bool isMajor = (i % 5 == 0);
        Vector4 currentColor = isMajor ? majorColor : regularColor;

        points_.emplace_back(Vector4(p, 0.0f, -half, 1.0f), currentColor);
        points_.emplace_back(Vector4(p, 0.0f, half, 1.0f), currentColor);
    }

    for (int i = 0; i <= divisionsAmount; ++i)
    {
        float p = -half + i * step;
        bool isMajor = (i % 5 == 0);
        Vector4 currentColor = isMajor ? majorColor : regularColor;

        points_.emplace_back(Vector4(-half, 0.0f, p, 1.0f), currentColor);
        points_.emplace_back(Vector4(half, 0.0f, p, 1.0f), currentColor);
    }
}
