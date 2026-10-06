
#include "Game.h"
#include "PaddleComponent.h"

using namespace std;

void PaddleComponent::update()
{

	float deltaY = 0.0f;
	float deltaX = 0.0f;

	if (game->inputDevice->IsKeyDown(moveUpKey)) 
	{
		deltaY = movementSpeed;
	}
	else if (game->inputDevice->IsKeyDown(moveDownKey)) 
	{
		deltaY = -movementSpeed;
	}
	else if (game->inputDevice->IsKeyDown(moveRightKey))
	{
		deltaX = movementSpeed;
	}
	else if (game->inputDevice->IsKeyDown(moveLeftKey))
	{
		deltaX = -movementSpeed;
	}

	if (deltaY != 0.0f)
	{

		float currentMinY = points_[0].y;
		float currentMaxY = points_[0].y;
		for (size_t i = 0; i < points_.size(); i += 2)
		{
			currentMinY = (std::min)(currentMinY, points_[i].y);
			currentMaxY = (std::max)(currentMaxY, points_[i].y);
		}

		const float epsilon = 0.001f;
		float nextMinY = currentMinY + deltaY;
		float nextMaxY = currentMaxY + deltaY;

		bool canMove = (nextMaxY <= 1.0f + epsilon) && (nextMinY >= -1.0f - epsilon);

		if (canMove)
		{
			for (size_t i = 0; i < points_.size(); i += 2)
			{
				points_[i].y += deltaY;
			}
		}
	}

	if (deltaX != 0.0f)
	{

		float currentMinX = points_[0].x;
		float currentMaxX = points_[0].x;
		for (size_t i = 0; i < points_.size(); i += 2)
		{
			currentMinX = (std::min)(currentMinX, points_[i].x);
			currentMaxX = (std::max)(currentMaxX, points_[i].x);
		}

		const float epsilon = 0.001f;
		float nextMinX = currentMinX + deltaX;
		float nextMaxX = currentMaxX + deltaX;

		bool canMove = (nextMaxX <= 1.0f + epsilon) && (nextMinX >= -1.0f - epsilon);

		if (canMove)
		{
			for (size_t i = 0; i < points_.size(); i += 2)
			{
				points_[i].x += deltaX;
			}
		}
	}
	TriangleComponent::update();
}

void::PaddleComponent::reset()
{

	resetHeight();

	//leftX = minX;
	//rightX = maxX;
	//centerX = (minX + maxX) * 0.5f;

	float currentMinX = points_[0].x;
	float currentMaxX = points_[0].x;

	for (size_t i = 0; i < points_.size(); i += 2) 
	{
		currentMinX = (std::min)(currentMinX, points_[i].x);
		currentMaxX = (std::max)(currentMaxX, points_[i].x);
	}

	float currentCenterX = (currentMinX + currentMaxX) * 0.5f;

	for (size_t i = 0; i < points_.size(); i += 2)
	{
		points_[i].x = (points_[i].x <= currentCenterX) ? leftX : rightX;
	}
	
}

void PaddleComponent::setKeys(Keys moveUp, Keys moveDown, Keys moveLeft, Keys moveRight) {
	this->moveUpKey = moveUp;
	this->moveDownKey = moveDown;
	this->moveLeftKey = moveLeft;
	this->moveRightKey = moveRight;
}

void PaddleComponent::updateHeight(float newHeight)
{	
	float minH = startHeight * 0.15f;
	currentHeight = std::clamp(newHeight, minH, startHeight);

	float halfH = currentHeight * 0.5f;

	float currentMinY = points_[0].y;
	float currentMaxY = points_[0].y;
	for (size_t i = 0; i < points_.size(); i += 2)
	{
		currentMinY = (std::min)(currentMinY, points_[i].y);
		currentMaxY = (std::max)(currentMaxY, points_[i].y);
	}
	float currentCenterY = (currentMinY + currentMaxY) * 0.5f;

	float topY = currentCenterY + halfH;
	float bottomY = currentCenterY - halfH;

	for (size_t i = 0; i < points_.size(); i += 2)
	{
		points_[i].y = (points_[i].y <= currentCenterY) ? bottomY : topY;
	}
}

void PaddleComponent::resetHeight()
{
	currentHeight = startHeight;
	float halfH = startHeight * 0.5f;

	float topY = centerY + halfH;
	float bottomY = centerY - halfH;

	float currentMinY = points_[0].y;
	float currentMaxY = points_[0].y;

	for (size_t i = 0; i < points_.size(); i += 2)
	{
		currentMinY = (std::min)(currentMinY, points_[i].y);
		currentMaxY = (std::max)(currentMaxY, points_[i].y);
	}

	float currentCenterY = (currentMinY + currentMaxY) * 0.5f;


	for (size_t i = 0; i < points_.size(); i += 2)
	{
		points_[i].y = (points_[i].y <= currentCenterY) ? bottomY : topY;
	}
}

std::vector<DirectX::XMFLOAT4> PaddleComponent::getPaddlePoints(float minX, float minY, float maxX, float maxY)
{
	return
	{
		DirectX::XMFLOAT4(minX,  minY, 0.0f, 1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		DirectX::XMFLOAT4(minX,  maxY,    0.0f, 1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		DirectX::XMFLOAT4(maxX, minY, 0.0f, 1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		DirectX::XMFLOAT4(maxX, maxY,    0.0f, 1.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
	};
}
std::vector<int> PaddleComponent::getPaddleIndeces()
{
	return { 0, 1, 2, 2, 1, 3 };
}