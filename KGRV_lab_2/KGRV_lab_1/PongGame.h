#pragma once

#include "Game.h"
#include "GameComponent.h"
#include "TriangleComponent.h"
#include "BallComponent.h"
#include "PaddleComponent.h"
#include "DisplayWin32.h"
#include "InputDevice.h"

enum class PongState 
{
	Normal,
	Cooldown,
	GameOver
};

class PongGame :
	public Game
{ 
public:

	PongGame(LPCWSTR applicationName, int screenHeight, int screenWidth);
	~PongGame();

	PongState state = PongState::Normal;
	std::chrono::steady_clock::time_point continueTime;

	void initialize() override;
	void update() override;
	void draw() override;
	void destroyResources() override;

	void addScore(bool isRightPlayer);
	void checkWinCondition();

private:

	int leftScore = 0;
	int rightScore = 0;
	const int WIN_SCORE = 5;

	PaddleComponent* leftPaddle;
	PaddleComponent* rightPaddle;
	BallComponent* ball;

};
