#pragma once

#include <d3d.h>
#include <d3d11.h>
#include "SimpleMath.h"
#include "GameComponent.h"

class Game;

class GridComponent :
	public GameComponent
{
public:

	struct Point
	{
		DirectX::SimpleMath::Vector4 position;
		DirectX::SimpleMath::Vector4 color;
	};

	GridComponent(Game* game, float size, int divisionsAmount);
	~GridComponent();

	float size;
	int divisionsAmount;

	std::vector<Point> points_ = {};

	void destroyResources() override;
	void draw() override;
	void initialize() override;
	void update() override;
	void reload() override;

	void reset();

private:

	ID3D11InputLayout* layout;

	ID3D11PixelShader* pixelShader;
	ID3DBlob* pixelShaderByteCode;

	ID3D11RasterizerState* rastState;

	ID3D11VertexShader* vertexShader;
	ID3DBlob* vertexShaderByteCode;

	ID3D11Buffer* vb;
	ID3D11Buffer* cb;

	UINT strides[1];
	UINT offsets[1];

};
