#pragma once

#include "SimpleMath.h"
#include "GameComponent.h"
#include "BodyComponent.h"

class Game;

class PlanetComponent :
	public BodyComponent
{
public:

	PlanetComponent(Game* game, float orbitRadius, float orbitSpeed, PlanetComponent* orbitCenter, float planetRadius, DirectX::SimpleMath::Vector4 planetColor);
	float selfRotationSpeed;
	void update() override;

private:

	float orbitRadius;
	float orbitSpeed;
	PlanetComponent* orbitCenter;
	DirectX::SimpleMath::Vector3 orbitAxis = DirectX::SimpleMath::Vector3::Up;
	float orbitAngle = 0.0f;
	DirectX::SimpleMath::Vector3 selfRotationAxis;

};
