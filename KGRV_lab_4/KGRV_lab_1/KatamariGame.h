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

#include "SimpleMath.h"
#include "KatamariComponent.h"
#include "PickupComponent.h"
#include "GridComponent.h"
#include "Game.h"

class KatamariGame :
	public Game
{
public:

	KatamariGame(LPCWSTR applicationName, int screenHeight, int screenWidth);
	~KatamariGame();

	void initialize() override;
	void update() override;
	void draw() override;
	void destroyResources() override;

	void spawnPickups();
	void checkCollisions();

private:

	GridComponent* grid;
	KatamariComponent* katamari;
	std::vector<PickupComponent*> pickups;
	std::unordered_set<PickupComponent*> collectedPickups;

};
