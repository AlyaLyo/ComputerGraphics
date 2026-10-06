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
#include <DDSTextureLoader.h>
#include "SimpleMath.h"
#include "GameComponent.h"

class Game;

class SkyboxComponent :
    public GameComponent
{
public:

    struct Point
    {
        DirectX::SimpleMath::Vector4 position;
    };

    struct CBPerObject
    {
        DirectX::SimpleMath::Matrix worldViewProj;
        DirectX::SimpleMath::Matrix world;
    };

    std::vector<Point> points_ = {};
    std::vector<int> indeces_ = {};

    SkyboxComponent(Game* game, std::wstring cubemapPath);
    ~SkyboxComponent();

    void destroyResources() override;
    void draw() override;
    void initialize() override;
    void reload() override;
    void update() override;
    void prepareFrame() override {}

private:

    ID3D11InputLayout* layout = nullptr;

    ID3D11PixelShader* pixelShader = nullptr;
    ID3DBlob* pixelShaderByteCode = nullptr;

    ID3D11VertexShader* vertexShader = nullptr;
    ID3DBlob* vertexShaderByteCode = nullptr;

    ID3D11RasterizerState* rastState = nullptr;
    ID3D11DepthStencilState* depthState = nullptr;

    ID3D11Buffer* ib = nullptr;
    ID3D11Buffer* vb = nullptr;
    ID3D11Buffer* cb = nullptr;

    ID3D11ShaderResourceView* cubemapSRV = nullptr;
    ID3D11SamplerState* samplerState = nullptr;

    UINT strides[1];
    UINT offsets[1];

    std::wstring cubemapPath;

    void generateCubeGeometry();
};
