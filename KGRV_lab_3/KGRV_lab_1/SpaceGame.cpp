#include "SpaceGame.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

SpaceGame::SpaceGame(LPCWSTR applicationName, int screenHeight, int screenWidth) :
	Game(applicationName, screenHeight, screenWidth)
{
}

SpaceGame::~SpaceGame()
{
}

void SpaceGame::initialize()
{
    grid = new GridComponent(this, 50.0f, 100);

    auto sun = new PlanetComponent(
        this,
        0.0f, 0.0f,           
        nullptr,              
        2.0f,                 
        Vector4(1.0f, 0.8f, 0.2f, 1.0f)
    );
    bodies.push_back(sun);

    auto mercury = new PlanetComponent(
        this,
        6.0f, 2.5f, 
        sun,                
        0.3f,                           
        Vector4(0.6f, 0.5f, 0.5f, 1.0f)
    );
    bodies.push_back(mercury);

    auto venus = new PlanetComponent(
        this,
        9.0f, 1.8f, 
        sun,
        0.6f,
        Vector4(0.9f, 0.7f, 0.3f, 1.0f)
    );
    bodies.push_back(venus);

    auto earth = new PlanetComponent(
        this,
        10.0f,                
        1.0f,                 
        sun,                  
        0.5f,                 
        Vector4(0.2f, 0.4f, 1.0f, 1.0f)
    );
    earth->selfRotationSpeed = 5;
    bodies.push_back(earth);

    auto moon = new PlanetComponent(
        this,
        2.0f,                 
        3.0f,                 
        earth,                
        0.15f,               
        Vector4(0.7f, 0.7f, 0.7f, 1.0f)
    );
    bodies.push_back(moon);

    auto mars = new PlanetComponent(
        this,
        17.0f, 0.9f, 
        sun,
        0.5f,
        Vector4(0.8f, 0.3f, 0.2f, 1.0f)
    );
    bodies.push_back(mars);

    auto phobos = new PlanetComponent(
        this,
        1.2f, 3.5f, 
        mars,
        0.08f,
        Vector4(0.5f, 0.4f, 0.4f, 1.0f)
    );
    bodies.push_back(phobos);

    auto jupiter = new PlanetComponent(
        this,
        24.0f, 0.5f, 
        sun,
        1.8f,                            
        Vector4(0.8f, 0.6f, 0.4f, 1.0f)
    );
    bodies.push_back(jupiter);

    auto io = new PlanetComponent(
        this,
        3.0f, 2.0f, 
        jupiter,
        0.25f,
        Vector4(0.9f, 0.8f, 0.1f, 1.0f)
    );
    bodies.push_back(io);

    auto europa = new PlanetComponent(
        this,
        4.5f, 1.5f, 
        jupiter,
        0.22f,
        Vector4(0.9f, 0.95f, 1.0f, 1.0f)
    );
    bodies.push_back(europa);

    auto saturn = new PlanetComponent(
        this,
        32.0f, 0.35f,
        sun,
        1.5f,
        Vector4(0.9f, 0.8f, 0.6f, 1.0f)
    );
    bodies.push_back(saturn);

    auto titan = new PlanetComponent(
        this,
        3.5f, 1.2f, 
        saturn,
        0.3f,
        Vector4(0.8f, 0.7f, 0.3f, 1.0f)
    );
    bodies.push_back(titan);

    Game::initialize();
}

void SpaceGame::update()
{
    Game::update();
}

void SpaceGame::draw()
{
    Game::draw();
}

void SpaceGame::destroyResources()
{
    Game::destroyResources();
}