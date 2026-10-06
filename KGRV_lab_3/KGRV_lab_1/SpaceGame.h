#pragma once

#include "SimpleMath.h"
#include "Game.h"
#include "Camera.h"
#include "GameComponent.h"
#include "BodyComponent.h"
#include "PlanetComponent.h"
#include "GridComponent.h"
#include "InputDevice.h"


class SpaceGame :
	public Game
{
public:

	SpaceGame(LPCWSTR applicationName, int screenHeight, int screenWidth);
	~SpaceGame();

	void initialize() override;
	void update() override;
	void draw() override;
	void destroyResources() override;

private:

	std::vector<BodyComponent*> bodies;
	GridComponent* grid;
	Camera* camera;

};