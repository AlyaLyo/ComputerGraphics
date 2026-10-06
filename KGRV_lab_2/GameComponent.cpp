#include "Game.h"
#include "GameComponent.h"

GameComponent::GameComponent(Game* game) :
	game(game)
{}

GameComponent::~GameComponent()
{}

GameComponent::AABB GameComponent::getAABB() const
{
	return { 0, 0, 0, 0 };
}