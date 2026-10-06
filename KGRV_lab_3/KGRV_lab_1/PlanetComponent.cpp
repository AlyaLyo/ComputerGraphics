#include "PlanetComponent.h"
#include "Game.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

PlanetComponent::PlanetComponent(Game* game, float orbitRadius, float orbitSpeed, PlanetComponent* orbitCenter, float planetRadius, DirectX::SimpleMath::Vector4 planetColor) :
	BodyComponent(game, planetRadius, planetColor),
	orbitRadius(orbitRadius),
	orbitSpeed(orbitSpeed),
	orbitCenter(orbitCenter),
	orbitAxis(Vector3::Up),
	orbitAngle(0.0f),
	selfRotationSpeed(0.5f),
	selfRotationAxis(Vector3::Up)
{
}

void PlanetComponent::update()
{
	float dt = game->deltaTime;

	Quaternion rotationDelta = Quaternion::CreateFromAxisAngle(selfRotationAxis, selfRotationSpeed * dt);
	rotation = rotation * rotationDelta;

	if (orbitCenter != nullptr)
	{
		orbitAngle += orbitSpeed * dt;
		Vector3 orbitalOffset;
		if (orbitAxis == Vector3::Up)
		{
			orbitalOffset.x = orbitRadius * sinf(orbitAngle);
			orbitalOffset.y = 0.0f;
			orbitalOffset.z = orbitRadius * cosf(orbitAngle);
		}
		else
		{
			Quaternion orbitRot = Quaternion::CreateFromAxisAngle(orbitAxis, orbitAngle);
			Vector3 baseOffset(orbitRadius, 0, 0);
			orbitalOffset = Vector3::Transform(baseOffset, orbitRot);
		}
		position = orbitCenter->position + orbitalOffset;
	}
	BodyComponent::update();
}