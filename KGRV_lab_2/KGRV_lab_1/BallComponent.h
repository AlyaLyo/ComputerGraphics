#pragma once

#include <algorithm>

#include "SimpleMath.h"

#include "GameComponent.h"
#include "TriangleComponent.h"
#include "PaddleComponent.h"

class Game;

class BallComponent :
	public TriangleComponent
{

public:

	BallComponent(Game* game);

	float movementSpeed;

	void update() override;
	void setDirection();
	void reset();

private:

	static constexpr float MAX_BOUNCE_ANGLE = 0.785398f; // pi/4
	static constexpr float SPEED_BOOST = 0.05f;
	static constexpr float MAX_SPEED = 3.0f;

	std::vector<DirectX::XMFLOAT4> ballPoints_ = 
	{ DirectX::XMFLOAT4(0, 0, 0, 1), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		DirectX::XMFLOAT4(0, 0, 0, 1), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		DirectX::XMFLOAT4(0, 0, 0, 1), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		DirectX::XMFLOAT4(0, 0, 0, 1), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 
	};
	std::vector<int> ballIndeces_ = { 0, 1, 2, 2, 1, 3 };

	float startX = 0.0f;
	float startY = 0.0f;

	float centerX;
	float centerY;

	float radius;
	DirectX::SimpleMath::Vector2 movementDirection;

};


