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
#include "SimpleMath.h"
#include "GameComponent.h"

class Game;

class BodyComponent :
	public GameComponent
{
public:



	struct Point
	{
		DirectX::SimpleMath::Vector4 position;
		DirectX::SimpleMath::Vector4 color;
		DirectX::SimpleMath::Vector4 normal;
	};

	struct CBPerObject
	{
		DirectX::SimpleMath::Matrix worldViewProjection;
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

	BodyComponent(Game* game, float radius, DirectX::SimpleMath::Vector4 color);
	~BodyComponent();

	std::vector<Point> points_ = {};
	std::vector<int> indeces_ = {};

	float scale = 1.0f;
	float radius;
	DirectX::SimpleMath::Vector4 color;
	DirectX::SimpleMath::Vector4 color2;
	DirectX::SimpleMath::Vector3 position;
	DirectX::SimpleMath::Quaternion rotation;

	DirectX::SimpleMath::Vector3 materialAmbient;
	DirectX::SimpleMath::Vector3 materialSpecular;
	float materialShininess;

	DirectX::SimpleMath::Vector3 getPosition()
	{
		return position;
	}

	void destroyResources() override;
	void draw() override;
	void initialize() override;
	void update() override;
	void reload() override;

	void generateSphereGeometry();

private:

	ID3D11InputLayout* layout;

	ID3D11PixelShader* pixelShader;
	ID3DBlob* pixelShaderByteCode;

	ID3D11RasterizerState* rastState;

	ID3D11VertexShader* vertexShader;
	ID3DBlob* vertexShaderByteCode;

	ID3D11Buffer* ib;
	ID3D11Buffer* vb;
	ID3D11Buffer* cb;
	ID3D11Buffer* cbLight;

	UINT strides[1];
	UINT offsets[1];

};
