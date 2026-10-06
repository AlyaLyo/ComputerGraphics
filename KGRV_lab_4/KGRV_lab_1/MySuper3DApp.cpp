#include "KatamariGame.h"

int main()
{
	KatamariGame game = KatamariGame(L"Katamari", 800, 800);
	game.run();
	std::cout << "Hello World!\n";
}

