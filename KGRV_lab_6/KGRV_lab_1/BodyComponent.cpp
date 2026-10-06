#include "BodyComponent.h"
#include "Game.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

BodyComponent::BodyComponent(Game* game, float radius, Vector4 color) :
    GameComponent(game),
    radius(radius),
    color(color),
    color2(color),
    materialAmbient(0.15f, 0.15f, 0.15f),
    materialSpecular(0.5f, 0.5f, 0.5f),
    materialShininess(32.0f)
{
    generateSphereGeometry();
    game->components.push_back(this);
}

BodyComponent::~BodyComponent()
{
}

void BodyComponent::initialize()
{
    D3D_SHADER_MACRO PhongMacro[] = { "PHONG_LIGHTING", "1", nullptr, nullptr };

    ID3DBlob* errorVertexCode = nullptr;
    auto res = D3DCompileFromFile(L"./Shaders/MyVeryFirstShader.hlsl",
        PhongMacro /*macros*/,
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

    ID3DBlob* errorPixelCode;
    res = D3DCompileFromFile(L"./Shaders/MyVeryFirstShader.hlsl",
        PhongMacro /*macros*/,
        nullptr /*include*/, "PSMain",
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

    // Load CSM shaders
    ID3DBlob* errorCsmVsCode = nullptr;
    res = D3DCompileFromFile(L"./Shaders/Csm.hlsl",
        nullptr,
        nullptr,
        "VSMain",
        "vs_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        &csmVertexShaderByteCode,
        &errorCsmVsCode);

    if (FAILED(res)) {
        if (errorCsmVsCode) {
            char* compileErrors = (char*)(errorCsmVsCode->GetBufferPointer());
            std::cout << compileErrors << std::endl;
        }
    }

    ID3DBlob* errorCsmGsCode = nullptr;
    res = D3DCompileFromFile(L"./Shaders/Csm.hlsl",
        nullptr,
        nullptr,
        "GSMain",
        "gs_5_0",
        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
        0,
        &csmGeometryShaderByteCode,
        &errorCsmGsCode);

    if (FAILED(res)) {
        if (errorCsmGsCode) {
            char* compileErrors = (char*)(errorCsmGsCode->GetBufferPointer());
            std::cout << compileErrors << std::endl;
        }
    }

    game->device->CreateVertexShader(
        csmVertexShaderByteCode->GetBufferPointer(),
        csmVertexShaderByteCode->GetBufferSize(),
        nullptr, &csmVertexShader);

    game->device->CreateGeometryShader(
        csmGeometryShaderByteCode->GetBufferPointer(),
        csmGeometryShaderByteCode->GetBufferSize(),
        nullptr, &csmGeometryShader);

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
            0},
     D3D11_INPUT_ELEMENT_DESC {
            "NORMAL",
            0,
            DXGI_FORMAT_R32G32B32A32_FLOAT,
            0,
            D3D11_APPEND_ALIGNED_ELEMENT,
            D3D11_INPUT_PER_VERTEX_DATA,
            0}
    };

    game->device->CreateInputLayout(
        inputElements,
        3,
        vertexShaderByteCode->GetBufferPointer(),
        vertexShaderByteCode->GetBufferSize(),
        &layout);

    D3D11_BUFFER_DESC vertexBufDesc = {};
    vertexBufDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexBufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vertexBufDesc.MiscFlags = 0;
    vertexBufDesc.StructureByteStride = 0;
    vertexBufDesc.ByteWidth = sizeof(Point) * std::size(points_);

    D3D11_SUBRESOURCE_DATA vertexData = {};
    vertexData.pSysMem = points_.data();
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    game->device->CreateBuffer(&vertexBufDesc, &vertexData, &vb);

    D3D11_BUFFER_DESC indexBufDesc = {};
    indexBufDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufDesc.CPUAccessFlags = 0;
    indexBufDesc.MiscFlags = 0;
    indexBufDesc.StructureByteStride = 0;
    indexBufDesc.ByteWidth = sizeof(int) * std::size(indeces_);

    D3D11_SUBRESOURCE_DATA indexData = {};
    indexData.pSysMem = indeces_.data();
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    game->device->CreateBuffer(&indexBufDesc, &indexData, &ib);

    strides[0] = 48;
    offsets[0] = 0;

    D3D11_BUFFER_DESC constBufDesc = {};
    constBufDesc.Usage = D3D11_USAGE_DEFAULT;
    constBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constBufDesc.CPUAccessFlags = 0;
    constBufDesc.MiscFlags = 0;
    constBufDesc.StructureByteStride = 0;
    constBufDesc.ByteWidth = sizeof(CBPerObject);

    game->device->CreateBuffer(&constBufDesc, nullptr, &cb);

    // Cascade constant buffer
    D3D11_BUFFER_DESC cascadeBufDesc = {};
    cascadeBufDesc.Usage = D3D11_USAGE_DEFAULT;
    cascadeBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cascadeBufDesc.CPUAccessFlags = 0;
    cascadeBufDesc.MiscFlags = 0;
    cascadeBufDesc.StructureByteStride = 0;
    cascadeBufDesc.ByteWidth = sizeof(CBCascade);

    game->device->CreateBuffer(&cascadeBufDesc, nullptr, &cbCascade);

    // Main rasterizer state
    CD3D11_RASTERIZER_DESC rastDesc = {};
    rastDesc.CullMode = D3D11_CULL_NONE;
    rastDesc.FillMode = D3D11_FILL_SOLID;
    rastDesc.FrontCounterClockwise = false;
    rastDesc.DepthClipEnable = true;

    res = game->device->CreateRasterizerState(&rastDesc, &rastState);

    // Shadow rasterizer state
    CD3D11_RASTERIZER_DESC shadowRastDesc = {};
    shadowRastDesc.CullMode = D3D11_CULL_FRONT;
    shadowRastDesc.FillMode = D3D11_FILL_SOLID;
    shadowRastDesc.FrontCounterClockwise = false;
    shadowRastDesc.DepthClipEnable = false;

    res = game->device->CreateRasterizerState(&shadowRastDesc, &shadowRastState);

    // Depth comparison sampler for shadow mapping
    D3D11_SAMPLER_DESC depthSamplerDesc = {};
    depthSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
    depthSamplerDesc.ComparisonFunc = D3D11_COMPARISON_GREATER_EQUAL;
    depthSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    depthSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    depthSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    depthSamplerDesc.BorderColor[0] = 1.0f;
    depthSamplerDesc.BorderColor[1] = 1.0f;
    depthSamplerDesc.BorderColor[2] = 1.0f;
    depthSamplerDesc.BorderColor[3] = 1.0f;

    res = game->device->CreateSamplerState(&depthSamplerDesc, &depthSamplerState);
}

void BodyComponent::prepareFrame()
{
    game->context->RSSetState(shadowRastState);

    D3D11_VIEWPORT viewport = {};
    viewport.Width = 1024.0f;
    viewport.Height = 1024.0f;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.MinDepth = 0;
    viewport.MaxDepth = 1.0f;

    game->context->RSSetViewports(1, &viewport);

    game->context->IASetInputLayout(layout);
    game->context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    game->context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
    game->context->IASetVertexBuffers(0, 1, &vb, strides, offsets);
    game->context->VSSetShader(csmVertexShader, nullptr, 0);
    game->context->VSSetConstantBuffers(0, 1, &cb);
    game->context->PSSetShader(nullptr, nullptr, 0);
    game->context->GSSetShader(csmGeometryShader, nullptr, 0);
    game->context->GSSetConstantBuffers(0, 1, &cbCascade);

    game->context->DrawIndexed(indeces_.size(), 0, 0);

    // Clean up geometry shader
    game->context->GSSetShader(nullptr, nullptr, 0);
}

void BodyComponent::draw()
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
    game->context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    game->context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
    game->context->IASetVertexBuffers(0, 1, &vb, strides, offsets);
    game->context->VSSetShader(vertexShader, nullptr, 0);
    game->context->VSSetConstantBuffers(0, 1, &cb);
    game->context->PSSetShader(pixelShader, nullptr, 0);
    game->context->PSSetConstantBuffers(1, 1, &cbCascade);
    game->context->PSSetConstantBuffers(2, 1, &game->perSceneCBuffer);

    // Bind shadow map
    ID3D11ShaderResourceView* csmSrv = game->GetCsm();
    game->context->PSSetShaderResources(1, 1, &csmSrv);
    game->context->PSSetSamplers(1, 1, &depthSamplerState);

    game->context->DrawIndexed(indeces_.size(), 0, 0);

    // Unbind shadow map SRV to avoid D3D warnings
    ID3D11ShaderResourceView* nullSrv = nullptr;
    game->context->PSSetShaderResources(1, 1, &nullSrv);
}

void BodyComponent::destroyResources()
{
}

void BodyComponent::update()
{
    const Matrix world = Matrix::CreateScale(scale) * Matrix::CreateFromQuaternion(rotation) * Matrix::CreateTranslation(position);

    CBPerObject cbData;
    cbData.worldViewProjection = (world * game->camera->getMatrix()).Transpose();
    cbData.world = world.Transpose();
    cbData.worldView = (world * game->camera->GetView()).Transpose();
    cbData.invTrWorldView = (Matrix::CreateScale(scale) * Matrix::CreateFromQuaternion(rotation)).Invert().Transpose() * game->camera->GetView().Transpose();
    game->context->UpdateSubresource(cb, 0, nullptr, &cbData, 0, 0);

    // Update cascade data
    CBCascade cascadeData = {};
    auto lightMatrices = game->GetDLight()->GetLightSpaceMatrices();
    for (int i = 0; i < 5; ++i)
    {
        cascadeData.viewProj[i] = lightMatrices[i].Transpose();
    }
    cascadeData.distance = game->GetDLight()->GetShadowCascadeDistances();
    game->context->UpdateSubresource(cbCascade, 0, nullptr, &cascadeData, 0, 0);
}

void BodyComponent::reload()
{
}

void BodyComponent::generateSphereGeometry()
{
    //Point topPoint({ Vector4(0.0f, radius, 0.0f, 1.0f), color });
    Point topPoint;
    topPoint.position = Vector4(0.0f, radius, 0.0f, 1.0f);
    topPoint.color = color;
    topPoint.normal = Vector4(0.0f, 1.0f, 0.0f, 0.0f);


    //Point bottomPoint({ Vector4(0.0f, -radius, 0.0f, 1.0f), color });
    Point bottomPoint;
    bottomPoint.position = Vector4(0.0f, -radius, 0.0f, 1.0f);
    bottomPoint.color = color;
    bottomPoint.normal = Vector4(0.0f, -1.0f, 0.0f, 0.0f);

    points_.push_back(topPoint);

    int sliceCount = 32;
    int stackCount = 32;
    const float phiStep = XM_PI / static_cast<float>(stackCount);
    const float thetaStep = XM_2PI / static_cast<float>(sliceCount);

    for (int i = 1; i <= stackCount - 1; ++i)
    {
        const float phi = static_cast<float>(i) * phiStep;

        for (int j = 0; j <= sliceCount; ++j)
        {
            const float theta = static_cast<float>(j) * thetaStep;
            Point point;

            float nx = sinf(phi) * cosf(theta);
            float ny = cosf(phi);
            float nz = sinf(phi) * sinf(theta);

            point.position = Vector4(radius * nx, radius * ny, radius * nz, 1.0f);
            point.color = (cosf(theta) >= 0.0f) ? color : color2;
            point.normal = Vector4(nx, ny, nz, 0.0f);

            points_.push_back(point);
        }
    }

    points_.push_back(bottomPoint);

    for (int i = 1; i <= sliceCount; ++i)
    {
        indeces_.push_back(0);
        indeces_.push_back(i + 1);
        indeces_.push_back(i);
    }

    int baseIndex = 1;
    const int ringVertexCount = sliceCount + 1;
    for (int i = 0; i < stackCount - 2; ++i)
    {
        for (int j = 0; j < sliceCount; ++j)
        {
            indeces_.push_back(baseIndex + i * ringVertexCount + j);
            indeces_.push_back(baseIndex + i * ringVertexCount + j + 1);
            indeces_.push_back(baseIndex + (i + 1) * ringVertexCount + j);

            indeces_.push_back(baseIndex + (i + 1) * ringVertexCount + j);
            indeces_.push_back(baseIndex + i * ringVertexCount + j + 1);
            indeces_.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
        }
    }

    const int southPoleIndex = points_.size() - 1;
    baseIndex = southPoleIndex - ringVertexCount;

    for (int i = 0; i < sliceCount; ++i)
    {
        indeces_.push_back(southPoleIndex);
        indeces_.push_back(baseIndex + i);
        indeces_.push_back(baseIndex + i + 1);
    }
}
