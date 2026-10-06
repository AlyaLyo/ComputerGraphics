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

#include <d3d.h>
#include <d3d11.h>
#include <string>
#include <vector>
#include <fstream>
#include <DDSTextureLoader.h>
#include "SimpleMath.h"
#include "GameComponent.h"

class Game;

class TerrainComponent :
        public GameComponent
{
public:

    struct Point
    {
        DirectX::SimpleMath::Vector4 position;
        DirectX::SimpleMath::Vector4 uv;
        DirectX::SimpleMath::Vector4 normal;
    };

    struct CBPerObject
    {
        DirectX::SimpleMath::Matrix worldViewProj;
        DirectX::SimpleMath::Matrix world;
    };

    struct CBLight
    {
        DirectX::SimpleMath::Vector4 lightDirection;
        DirectX::SimpleMath::Vector4 lightColor;
        DirectX::SimpleMath::Vector4 cameraPosition;
        DirectX::SimpleMath::Vector4 materialParams;
        DirectX::SimpleMath::Vector4 materialSpecular;
    };

    TerrainComponent(Game* game, float size, int divisions, std::string name, float heightScale);
    ~TerrainComponent();

    std::vector<Point> points_ = {};
    std::vector<int> indeces_ = {};

    float size;
    int divisions;
    float heightScale;

    DirectX::SimpleMath::Vector3 materialAmbient;
    DirectX::SimpleMath::Vector3 materialSpecular;
    float materialShininess;

    float getHeightAt(float worldX, float worldZ) const;

    void destroyResources() override;
    void draw() override;
    void initialize() override;
    void update() override;
    void reload() override;

    void generateTerrain();
    void calculateNormals();

private:
        
    ID3D11InputLayout* layout = nullptr;

    ID3D11PixelShader* pixelShader = nullptr;
    ID3DBlob* pixelShaderByteCode = nullptr;

    ID3D11VertexShader* vertexShader = nullptr;
    ID3DBlob* vertexShaderByteCode = nullptr;

    ID3D11RasterizerState* rastState = nullptr;

    ID3D11Buffer* ib = nullptr;
    ID3D11Buffer* vb = nullptr;
    ID3D11Buffer* cb = nullptr;
    ID3D11Buffer* cbLight = nullptr;

    ID3D11ShaderResourceView* textureSRV = nullptr;
    ID3D11SamplerState* samplerState = nullptr;

    UINT strides[1];
    UINT offsets[1];

    std::wstring texturePath;
    std::wstring heightmapPath;

    std::vector<float> heightmapData;
    int heightmapWidth = 0;
    int heightmapHeight = 0;

    void loadTexture(const std::wstring& path);
    void loadHeightmapBMP(const std::wstring& path);
    float sampleHeightmap(float u, float v) const;
};
