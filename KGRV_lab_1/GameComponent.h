#pragma once

class Game;

class GameComponent
{

public:

	GameComponent(Game* game);
	virtual ~GameComponent();

	Game* game;

	virtual void destroyResources() = 0;
	virtual void draw() = 0;
	virtual void initialize() = 0;
	virtual void reload() = 0;
	virtual void update() = 0;

};
