#include "PongGame.h"

int main()
{
	PongGame pong = PongGame(L"Pong", 800, 800);
	pong.run();
	std::cout << "Hello World!\n";
}

