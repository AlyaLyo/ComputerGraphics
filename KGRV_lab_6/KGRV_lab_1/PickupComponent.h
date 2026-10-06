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
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <DDSTextureLoader.h>
#include "SimpleMath.h"
#include "GameComponent.h"

class Game;

class PickupComponent :
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
        DirectX::SimpleMath::Matrix worldView;
        DirectX::SimpleMath::Matrix invTrWorldView;
    };

    struct CBCascade
    {
        DirectX::SimpleMath::Matrix viewProj[5];
        DirectX::SimpleMath::Vector4 distance;
    };

    PickupComponent(Game* game, std::string name, float boundingRadius, DirectX::SimpleMath::Vector3 position);
    ~PickupComponent();

    std::string name;
    std::vector<Point> points_ = {};
    std::vector<int> indeces_ = {};
    DirectX::SimpleMath::Vector3 position;
    DirectX::SimpleMath::Quaternion rotation;
    float boundingRadius;
    float modelBottomY;

    DirectX::SimpleMath::Vector3 materialAmbient;
    DirectX::SimpleMath::Vector3 materialSpecular;
    float materialShininess;

    void destroyResources() override;
    void draw() override;
    void initialize() override;
    void update() override;
    void reload() override;
    void prepareFrame() override;

private:

    ID3D11InputLayout* layout;
    ID3D11ShaderResourceView* textureSRV;

    ID3D11PixelShader* pixelShader;
    ID3DBlob* pixelShaderByteCode;

    ID3D11VertexShader* vertexShader;
    ID3DBlob* vertexShaderByteCode;

    // CSM shadow pass shaders
    ID3D11VertexShader* csmVertexShader;
    ID3DBlob* csmVertexShaderByteCode;
    ID3D11GeometryShader* csmGeometryShader;
    ID3DBlob* csmGeometryShaderByteCode;

    ID3D11RasterizerState* rastState;
    ID3D11RasterizerState* shadowRastState;
    ID3D11SamplerState* samplerState;
    ID3D11SamplerState* depthSamplerState;

    ID3D11Buffer* ib;
    ID3D11Buffer* vb;
    ID3D11Buffer* cb;
    ID3D11Buffer* cbCascade;

    UINT strides[1];
    UINT offsets[1];

    std::string modelPath;
    std::wstring texturePath;

    void loadModel(const std::string& path);
    void loadTexture(const std::wstring& path);

};