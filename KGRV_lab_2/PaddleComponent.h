#pragma once

#include <algorithm>

#include "GameComponent.h"
#include "TriangleComponent.h"
#include "Keys.h"

class Game;

class PaddleComponent :
	public TriangleComponent
{
public:

	Keys moveUpKey;
	Keys moveDownKey;
	Keys moveLeftKey;
	Keys moveRightKey;

	float movementSpeed = 0.005f;
	float startHeight = 1.0f;
	float currentHeight = 1.0f;

	PaddleComponent(Game* game, float minX, float minY, float maxX, float maxY) :
		TriangleComponent(game,
			getPaddlePoints(minX, minY, maxX, maxY),
			getPaddleIndeces())
	{
		movementSpeed = 0.005f;
		leftX = minX;
		rightX = maxX;
		centerX = (minX + maxX) * 0.5f;
		centerY = (maxY + minY) * 0.5f;

		startHeight = maxY - minY;
		currentHeight = startHeight;
	}

	template<size_t N, size_t M>
	PaddleComponent(Game* game, const DirectX::XMFLOAT4(&srcPoints)[N], const uint32_t(&srcIndices)[M]) :
		TriangleComponent(game,
			std::vector<DirectX::XMFLOAT4>(srcPoints, srcPoints + N),
			std::vector<int>(srcIndices, srcIndices + M))
	{
	}

	void update() override;
	void reset();

	void setKeys(Keys moveUp, Keys moveDown, Keys moveLeft, Keys moveRight);
	void updateHeight(float newHeight);
	void resetHeight();

private:

	float centerX = 0.0f;
	float centerY = 0.0f;
	float leftX = 0.0f;
	float rightX = 0.0f;

	std::vector<DirectX::XMFLOAT4> getPaddlePoints(float minX, float minY, float maxX, float maxY);
	std::vector<int> getPaddleIndeces();

};