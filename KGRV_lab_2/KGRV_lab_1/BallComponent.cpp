
#include "Game.h"
#include "BallComponent.h"

using namespace DirectX::SimpleMath;
using namespace std;

BallComponent::BallComponent(Game* game) :
	TriangleComponent(
		game,
		{ DirectX::XMFLOAT4(0, 0, 0, 1), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		DirectX::XMFLOAT4(0, 0, 0, 1), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		DirectX::XMFLOAT4(0, 0, 0, 1), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		DirectX::XMFLOAT4(0, 0, 0, 1), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		},
		{ 0, 1, 2, 2, 1, 3 })
{
	startX = 0.0f; 
	startY = 0.0f;
	radius = 0.05f;
	reset();
	
	points_[0] = DirectX::XMFLOAT4(centerX - radius, centerY - radius, 0, 1);
	points_[2] = DirectX::XMFLOAT4(centerX - radius, centerY + radius, 0, 1);
	points_[4] = DirectX::XMFLOAT4(centerX + radius, centerY - radius, 0, 1);
	points_[6] = DirectX::XMFLOAT4(centerX + radius, centerY + radius, 0, 1);
	
}

void BallComponent::update()
{
	float nextX = centerX + movementDirection.x * movementSpeed * game->deltaTime;
	float nextY = centerY + movementDirection.y * movementSpeed * game->deltaTime;

	if (nextY + radius > 1.0f)
	{
		nextY = 1.0f - radius;
		movementDirection.y = -std::abs(movementDirection.y);
	}
	else if (centerY - radius < -1.0f)
	{
		nextY = -1.0f + radius;
		movementDirection.y = std::abs(movementDirection.y);
	}

	for (auto* component : game->components)
	{
		if (component == this)
		{
			continue;
		}

		const auto& box = component->getAABB();

		bool isMovingRight = movementDirection.x > 0;
		float paddleFrontX = isMovingRight ? box.minX : box.maxX;

		bool movingTowards = (isMovingRight && centerX < paddleFrontX) || (!isMovingRight && centerX > paddleFrontX);
		if (!movingTowards)
		{
			continue;
		}

		float nextX = centerX + movementDirection.x * movementSpeed * game->deltaTime;
		bool willCrossFront = (isMovingRight && nextX + radius >= paddleFrontX) || (!isMovingRight && nextX - radius <= paddleFrontX);
		if (!willCrossFront)
		{
			continue;
		}

		float timeToX = std::abs(paddleFrontX - centerX) / (movementSpeed * std::abs(movementDirection.x));
		float hitY = centerY + movementDirection.y * movementSpeed * game->deltaTime * timeToX;
		if (hitY + radius > box.minY && hitY - radius < box.maxY)
		{
			movementDirection.x = -movementDirection.x;

			float paddleCenterY = (box.minY + box.maxY) * 0.5f;
			float halfHeight = (box.maxY - box.minY) * 0.5f;
			float hitRel = std::clamp((hitY - paddleCenterY) / halfHeight, -1.0f, 1.0f);
			float angle = hitRel * MAX_BOUNCE_ANGLE;

			float dirX = isMovingRight ? -1.0f : 1.0f;
			movementDirection = DirectX::SimpleMath::Vector2(dirX * std::cos(angle), std::sin(angle));
			movementDirection.Normalize();

			movementSpeed = (std::min)(movementSpeed + SPEED_BOOST, MAX_SPEED);
			centerX = isMovingRight ? (paddleFrontX - radius - 0.001f) : (paddleFrontX + radius + 0.001f);
			centerY = hitY;

			auto* paddle = dynamic_cast<PaddleComponent*>(component);
//			paddle->updateHeight((std::max)(paddle->startHeight * 0.15f, paddle->currentHeight * 0.95f));
			paddle->updateHeight(paddle->currentHeight * 0.95f);

			break;
		}
	}

	if (nextX > 1.0f + radius || nextX < -1.0f - radius)
	{

		reset();
		for (auto* component : game->components)
		{
			if (component == this)
			{
				continue;
			}
			auto* paddle = dynamic_cast<PaddleComponent*>(component);
			paddle->reset();
		}
	
		return;
	}

	centerX = nextX;
	centerY = nextY;

	points_[0] = DirectX::XMFLOAT4(centerX - radius, centerY - radius, 0, 1);
	points_[2] = DirectX::XMFLOAT4(centerX - radius, centerY + radius, 0, 1);
	points_[4] = DirectX::XMFLOAT4(centerX + radius, centerY - radius, 0, 1);
	points_[6] = DirectX::XMFLOAT4(centerX + radius, centerY + radius, 0, 1);

	TriangleComponent::update();
}

void BallComponent::setDirection()
{
	DirectX::SimpleMath::Vector2::Lerp(DirectX::SimpleMath::Vector2(1.0f, 1.0f), DirectX::SimpleMath::Vector2(1.0f, -1.0f), (float)rand() / RAND_MAX, movementDirection);
	if ((float)rand() / RAND_MAX < 0.5f)
		movementDirection.x *= -1;
	movementDirection.Normalize();
}

void BallComponent::reset()
{
	centerX = startX;
	centerY = startY;

	movementSpeed = 0.7f;

	setDirection();
}