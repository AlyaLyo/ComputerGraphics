
#include "PongGame.h"

PongGame::PongGame(LPCWSTR applicationName, int screenHeight, int screenWidth) :
	Game(applicationName, screenHeight, screenWidth)
{
}

PongGame::~PongGame()
{
}

void PongGame::initialize()
{
	leftPaddle = new PaddleComponent(this, -0.95f, -0.2f, -0.90f, 0.2f);
	rightPaddle = new PaddleComponent(this, 0.90f, -0.2f, 0.95f, 0.2f);
	ball = new BallComponent(this);

	leftPaddle->setKeys(Keys::W, Keys::S, Keys::A, Keys::D);
	rightPaddle->setKeys(Keys::Up, Keys::Down, Keys::Left, Keys::Right);

	Game::initialize();
}

void PongGame::update()
{
	if (state == PongState::Cooldown)
	{
		if (std::chrono::steady_clock::now() >= continueTime)
		{
			state = PongState::Normal;
			if (ball) 
			{
				ball->reset();
			}
		}

		leftPaddle->update();
		rightPaddle->update();
		return;
	}

	Game::update();

}

void PongGame::draw()
{
	Game::draw();
}

void PongGame::destroyResources()
{
	Game::destroyResources();
}