#pragma once

#include "Game.h"
#include "GameComponent.h"

#include <d3d.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <vector>
#include <iostream>

//class Game;


class TriangleComponent :
	public GameComponent
{

public:

	std::vector<DirectX::XMFLOAT4> points_ = {};
	std::vector<int> indeces_ = {};
/*	
	struct AABB
	{
		float minX;
		float minY;
		float maxX;
		float maxY;
	};
*/

	TriangleComponent(Game* game, std::vector<DirectX::XMFLOAT4> points, std::vector<int> indeces) :
		GameComponent(game),
		points_(points),
		indeces_(indeces)
	{
		game->components.push_back(this);
	}

	template<size_t N, size_t M>
	TriangleComponent(Game* game, const DirectX::XMFLOAT4(&srcPoints)[N], const uint32_t(&srcIndices)[M]) :
		TriangleComponent(game,
			std::vector<DirectX::XMFLOAT4>(srcPoints, srcPoints + N),
			std::vector<int>(srcIndices, srcIndices + M))
	{
	}

	~TriangleComponent();

	void destroyResources() override;
	void draw() override;
	void initialize() override;
	void update() override;
	void reload() override;

	AABB getAABB() const override;

private:

	ID3D11InputLayout* layout;

	ID3D11PixelShader* pixelShader;
	ID3DBlob* pixelShaderByteCode;

	ID3D11RasterizerState* rastState;

	ID3D11VertexShader* vertexShader;
	ID3DBlob* vertexShaderByteCode;

	ID3D11Buffer* ib;
	ID3D11Buffer* vb;

	UINT strides[1];
	UINT offsets[1];



};