#include "TerrainComponent.h"
#include "Game.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

TerrainComponent::TerrainComponent(Game* game, float size, int divisions, string name, float heightScale) :
    GameComponent(game),
    size(size),
    divisions(divisions),
    heightScale(heightScale),

    materialAmbient(0.15f, 0.15f, 0.15f),
    materialSpecular(0.3f, 0.3f, 0.3f),
    materialShininess(16.0f)
{
    texturePath = L"./Textures/";
    texturePath.append(name.begin(), name.end());
    texturePath += L".dds";

    heightmapPath = L"./Textures/";
    heightmapPath.append(name.begin(), name.end());
    heightmapPath += L"Heightmap.bmp";

    loadHeightmapBMP(heightmapPath);
    generateTerrain();
    game->components.push_back(this);
}

TerrainComponent::~TerrainComponent()
{}

void TerrainComponent::initialize()
{
    ID3DBlob* errorVertexCode = nullptr;
    auto res = D3DCompileFromFile(L"./Shaders/TerrainShader.hlsl",
        nullptr,
        nullptr,
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
        else {
            MessageBox(game->display->hWnd, L"TerrainShader.hlsl", L"Missing Shader File", MB_OK);
        }
        return;
    }

    ID3DBlob* errorPixelCode = nullptr;
    res = D3DCompileFromFile(L"./Shaders/TerrainShader.hlsl",
        nullptr,
        nullptr,
        "PSMain",
        "ps_5_0",
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
            { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0,                            D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL",   0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    game->device->CreateInputLayout(
        inputElements,
        3,
        vertexShaderByteCode->GetBufferPointer(),
        vertexShaderByteCode->GetBufferSize(),
        &layout);

    loadTexture(texturePath);

    D3D11_BUFFER_DESC vertexBufDesc = {};
    vertexBufDesc.Usage = D3D11_USAGE_IMMUTABLE;
    vertexBufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufDesc.CPUAccessFlags = 0;
    vertexBufDesc.MiscFlags = 0;
    vertexBufDesc.StructureByteStride = 0;
    vertexBufDesc.ByteWidth = sizeof(Point) * static_cast<UINT>(points_.size());

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
    indexBufDesc.ByteWidth = sizeof(int) * static_cast<UINT>(indeces_.size());

    D3D11_SUBRESOURCE_DATA indexData = {};
    indexData.pSysMem = indeces_.data();
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    game->device->CreateBuffer(&indexBufDesc, &indexData, &ib);

    strides[0] = sizeof(Point);  // 3 * Vector4 = 48 bytes
    offsets[0] = 0;

    D3D11_BUFFER_DESC constBufDesc = {};
    constBufDesc.Usage = D3D11_USAGE_DEFAULT;
    constBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constBufDesc.CPUAccessFlags = 0;
    constBufDesc.MiscFlags = 0;
    constBufDesc.StructureByteStride = 0;
    constBufDesc.ByteWidth = sizeof(CBPerObject);

    game->device->CreateBuffer(&constBufDesc, nullptr, &cb);

    // ---- Constant Buffer: Light ----
    D3D11_BUFFER_DESC lightBufDesc = {};
    lightBufDesc.Usage = D3D11_USAGE_DEFAULT;
    lightBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    lightBufDesc.CPUAccessFlags = 0;
    lightBufDesc.MiscFlags = 0;
    lightBufDesc.StructureByteStride = 0;
    lightBufDesc.ByteWidth = sizeof(CBLight);

    game->device->CreateBuffer(&lightBufDesc, nullptr, &cbLight);

    CD3D11_RASTERIZER_DESC rastDesc = {};
    rastDesc.CullMode = D3D11_CULL_BACK;
    rastDesc.FillMode = D3D11_FILL_SOLID;
    rastDesc.FrontCounterClockwise = true;
    rastDesc.DepthClipEnable = true;

    game->device->CreateRasterizerState(&rastDesc, &rastState);

    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    game->device->CreateSamplerState(&samplerDesc, &samplerState);
}

void TerrainComponent::draw()
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
    game->context->PSSetConstantBuffers(1, 1, &cbLight);

    game->context->PSSetShaderResources(0, 1, &textureSRV);
    game->context->PSSetSamplers(0, 1, &samplerState);

    game->context->DrawIndexed(static_cast<UINT>(indeces_.size()), 0, 0);
}

void TerrainComponent::destroyResources()
{}

void TerrainComponent::update()
{
    Matrix world = Matrix::Identity;
    Matrix worldViewProj = world * game->camera->getMatrix();

    CBPerObject cbData;
    cbData.worldViewProj = worldViewProj.Transpose();
    cbData.world = world.Transpose();
    game->context->UpdateSubresource(cb, 0, nullptr, &cbData, 0, 0);

    CBLight lightData;
    Vector3 lightDir = Vector3(1.0f, 1.0f, 1.0f);
    lightDir.Normalize();
    lightData.lightDirection = Vector4(lightDir.x, lightDir.y, lightDir.z, 0.0f);

    lightData.lightColor = Vector4(1.0f, 0.95f, 0.9f, 0.0f);

    Vector3 camPos = game->camera->GetPosition();
    lightData.cameraPosition = Vector4(camPos.x, camPos.y, camPos.z, 0.0f);

    lightData.materialParams = Vector4(
        materialAmbient.x,
        materialAmbient.y,
        materialAmbient.z,
        materialShininess);

    lightData.materialSpecular = Vector4(
        materialSpecular.x,
        materialSpecular.y,
        materialSpecular.z,
        0.0f);

    game->context->UpdateSubresource(cbLight, 0, nullptr, &lightData, 0, 0);
}

void TerrainComponent::reload()
{}

void TerrainComponent::generateTerrain()
{
    points_.clear();
    indeces_.clear();

    float half = size * 0.5f;
    int vertCountPerRow = divisions + 1;
    float step = size / static_cast<float>(divisions);

    for (int z = 0; z <= divisions; ++z)
    {
        for (int x = 0; x <= divisions; ++x)
        {
            float px = -half + x * step;
            float pz = -half + z * step;

            float u = static_cast<float>(x) / divisions;
            float v = static_cast<float>(z) / divisions;

            float h = sampleHeightmap(u, v) * heightScale;

            Point p;
            p.position = Vector4(px, h, pz, 1.0f);
            p.uv = Vector4(u, v, 0.0f, 0.0f);
            p.normal = Vector4(0.0f, 1.0f, 0.0f, 0.0f);

            points_.push_back(p);
        }
    }

    for (int z = 0; z < divisions; ++z)
    {
        for (int x = 0; x < divisions; ++x)
        {
            int topLeft = z * vertCountPerRow + x;
            int topRight = topLeft + 1;
            int bottomLeft = topLeft + vertCountPerRow;
            int bottomRight = bottomLeft + 1;

            indeces_.push_back(topLeft);
            indeces_.push_back(bottomLeft);
            indeces_.push_back(topRight);

            indeces_.push_back(topRight);
            indeces_.push_back(bottomLeft);
            indeces_.push_back(bottomRight);
        }
    }

    calculateNormals();
}

void TerrainComponent::calculateNormals()
{
    for (auto& p : points_)
    {
        p.normal = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
    }

    for (size_t i = 0; i < indeces_.size(); i += 3)
    {
        int i0 = indeces_[i];
        int i1 = indeces_[i + 1];
        int i2 = indeces_[i + 2];

        Vector3 v0(points_[i0].position.x, points_[i0].position.y, points_[i0].position.z);
        Vector3 v1(points_[i1].position.x, points_[i1].position.y, points_[i1].position.z);
        Vector3 v2(points_[i2].position.x, points_[i2].position.y, points_[i2].position.z);

        Vector3 edge1 = v1 - v0;
        Vector3 edge2 = v2 - v0;
        Vector3 faceNormal = edge1.Cross(edge2);

        points_[i0].normal += Vector4(faceNormal.x, faceNormal.y, faceNormal.z, 0.0f);
        points_[i1].normal += Vector4(faceNormal.x, faceNormal.y, faceNormal.z, 0.0f);
        points_[i2].normal += Vector4(faceNormal.x, faceNormal.y, faceNormal.z, 0.0f);
    }

    for (auto& p : points_)
    {
        Vector3 n(p.normal.x, p.normal.y, p.normal.z);
        n.Normalize();
        p.normal = Vector4(n.x, n.y, n.z, 0.0f);
    }
}

float TerrainComponent::getHeightAt(float worldX, float worldZ) const
{
    if (heightmapData.empty())
    {
        return 0.0f;
    }

        float half = size * 0.5f;

        float u = (worldX + half) / size;
        float v = (worldZ + half) / size;

        return sampleHeightmap(u, v) * heightScale;
}

float TerrainComponent::sampleHeightmap(float u, float v) const
{
    if (heightmapData.empty())
    {
        return 0.0f;
    }

    u = std::max(0.0f, std::min(u, 1.0f));
    v = std::max(0.0f, std::min(v, 1.0f));

    float fx = u * (heightmapWidth - 1);
    float fy = v * (heightmapHeight - 1);

    int x0 = static_cast<int>(fx);
    int y0 = static_cast<int>(fy);
    int x1 = std::min(x0 + 1, heightmapWidth - 1);
    int y1 = std::min(y0 + 1, heightmapHeight - 1);

    float fracX = fx - static_cast<float>(x0);
    float fracY = fy - static_cast<float>(y0);

    float h00 = heightmapData[y0 * heightmapWidth + x0];
    float h10 = heightmapData[y0 * heightmapWidth + x1];
    float h01 = heightmapData[y1 * heightmapWidth + x0];
    float h11 = heightmapData[y1 * heightmapWidth + x1];

    float h0 = h00 * (1.0f - fracX) + h10 * fracX;
    float h1 = h01 * (1.0f - fracX) + h11 * fracX;

    return h0 * (1.0f - fracY) + h1 * fracY;
}

void TerrainComponent::loadHeightmapBMP(const std::wstring& path)
{
    ifstream file(path, std::ios::binary);
    if (!file.is_open())
    {
        std::wcerr << L"Heightmap was not found: " << path << std::endl;
        return;
    }

    uint8_t fileHeader[14];
    if (!file.read(reinterpret_cast<char*>(fileHeader), 14))
    {
        std::wcerr << L"Failed to read BMP header from: " << path << std::endl;
        return;
    }

    if (fileHeader[0] != 'B' || fileHeader[1] != 'M')
    {
        std::wcerr << L"Not a valid BMP file: " << path << std::endl;
        return;
    }

    uint32_t pixelDataOffset = *reinterpret_cast<uint32_t*>(&fileHeader[10]);
    uint8_t dibHeader[40];
    if (!file.read(reinterpret_cast<char*>(dibHeader), 40))
    {
        std::wcerr << L"Failed to read BMP DIB header from: " << path << std::endl;
        return;
    }

    int32_t width = *reinterpret_cast<int32_t*>(&dibHeader[4]);
    int32_t height = *reinterpret_cast<int32_t*>(&dibHeader[8]);
    uint16_t bpp = *reinterpret_cast<uint16_t*>(&dibHeader[14]);

    if (bpp != 8 && bpp != 24 && bpp != 32)
    {
        std::wcerr << L"Unsupported BMP bit depth (" << bpp << L") in: " << path << std::endl;
        return;
    }

    heightmapWidth = width;
    heightmapHeight = abs(height);
    bool bottomUp = (height > 0);

    int rowSize = ((bpp * width + 31) / 32) * 4;
    std::vector<uint8_t> rowData(rowSize);

    heightmapData.resize(heightmapWidth * heightmapHeight);

    file.seekg(pixelDataOffset);

    for (int y = 0; y < heightmapHeight; ++y)
    {
        if (!file.read(reinterpret_cast<char*>(rowData.data()), rowSize))
        {
            std::wcerr << L"Failed to read BMP row " << y << L" from: " << path << std::endl;
            heightmapData.clear();
            return;
        }

        int destY = bottomUp ? (heightmapHeight - 1 - y) : y;

        for (int x = 0; x < width; ++x)
        {
            float value = 0.0f;
            if (bpp == 8)
            {
                value = static_cast<float>(rowData[x]) / 255.0f;
            }
            else if (bpp == 24)
            {
                int offset = x * 3;
                float b = static_cast<float>(rowData[offset]);
                float g = static_cast<float>(rowData[offset + 1]);
                float r = static_cast<float>(rowData[offset + 2]);
                value = (r + g + b) / (3.0f * 255.0f);
            }
            else if (bpp == 32)
            {
                int offset = x * 4;
                float b = static_cast<float>(rowData[offset]);
                float g = static_cast<float>(rowData[offset + 1]);
                float r = static_cast<float>(rowData[offset + 2]);
                value = (r + g + b) / (3.0f * 255.0f);
            }

            heightmapData[destY * width + x] = value;
        }
    }

    std::wcout << L"Heightmap loaded: " << path << L" (" << heightmapWidth << L"x" << heightmapHeight << L", " << bpp << L"-bit)" << std::endl;
}

void TerrainComponent::loadTexture(const wstring& path)
{
    HRESULT hr = CreateDDSTextureFromFile(
        game->device.Get(),
        path.c_str(),
        nullptr,
        &textureSRV
    );

    if (FAILED(hr))
    {
        std::cerr << "Failed to load terrain texture: ";
        std::wcerr << path << L" HRESULT: 0x" << std::hex << hr << std::endl;
    }
    else
    {
        std::wcout << L"Terrain texture loaded: " << path << std::endl;
    }
}
