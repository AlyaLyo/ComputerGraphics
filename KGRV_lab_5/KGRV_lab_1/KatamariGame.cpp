#include "KatamariGame.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;
using namespace std;

KatamariGame::KatamariGame(LPCWSTR applicationName, int screenHeight, int screenWidth) :
	Game(applicationName, screenHeight, screenWidth)
{}

KatamariGame::~KatamariGame()
{}

void KatamariGame::initialize()
{
	//grid = new GridComponent(this, 50.0f, 100);
	terrain = new TerrainComponent(this, 50.0f, 200, "Terrain", 5.0f);

	katamari = new KatamariComponent(this, 0.3f, Vector4(0.1f, 0.85f, 0.2f, 1.0f));
	katamari->color2 = Vector4(0.85f, 0.15f, 0.1f, 1.0f);
	katamari->points_.clear();
	katamari->indeces_.clear();
	katamari->generateSphereGeometry();

	katamari->terrain = terrain;
	katamari->position = Vector3(0.0f, 0.0f, 0.0f);

	spawnPickups();
	Game::initialize();

	camera->setupTPCamera(katamari, 5.0f, 2.0f);
}

void KatamariGame::update()
{
	checkCollisions();
	Game::update();
}

void KatamariGame::draw()
{
	Game::draw();
}

void KatamariGame::destroyResources()
{
	Game::destroyResources();
}

void KatamariGame::spawnPickups()
{
	struct ModelInfo
	{
		std::string name;
		float minRadius;
		float maxRadius;
		int count;
	};

	vector<ModelInfo> modelTypes = {
		{ "Cake",              0.30f, 1.30f, 5 },
		{ "Crossaint",         0.20f, 0.40f, 5 },
		{ "Nuttella_Latte",    0.30f, 0.70f, 5 },
		{ "Starbucks-XmasCup", 0.30f, 0.70f, 5 },
		{ "Waffle",            0.20f, 0.40f, 5 },
	};

	srand(static_cast<unsigned>(std::time(nullptr)));
	float spawnRange = 25.0f;
	auto randFloat = [](float lo, float hi) -> float 
	{
		return lo + static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * (hi - lo);
	};

	for (const auto& model : modelTypes)
	{
		for (int i = 0; i < model.count; ++i)
		{

			float radius = randFloat(model.minRadius, model.maxRadius);

			float x = randFloat(-spawnRange, spawnRange);
			float z = randFloat(-spawnRange, spawnRange);

			float distFromCenter = sqrtf(x * x + z * z);
			float minDist = 3.0f + radius;
			if (distFromCenter < minDist)
			{
				if (distFromCenter < 0.001f)
				{
					x = minDist;
					z = 0.0f;
				}
				else
				{
					float scale = minDist / distFromCenter;
					x *= scale;
					z *= scale;
				}
			}

			float terrainY = terrain->getHeightAt(x, z);
			Vector3 worldPos(x, terrainY, z);

			auto pickup = new PickupComponent(
				this,
				model.name,
				radius,
				worldPos
			);

			Quaternion baseRot = Quaternion::Identity;
			if (model.name == "Starbucks-XmasCup")
			{
				baseRot = Quaternion::CreateFromAxisAngle(Vector3::Right, XM_PI);
			}
			else if (model.name == "Nuttella_Latte")
			{
				baseRot = Quaternion::CreateFromAxisAngle(Vector3::Right, XM_PIDIV2);
			}

			float randomYaw = randFloat(0.0f, XM_2PI);
			Quaternion yawRot = Quaternion::CreateFromAxisAngle(Vector3::Up, randomYaw);

			pickup->rotation = baseRot * yawRot;

			pickups.push_back(pickup);
		}
	}
}

void KatamariGame::checkCollisions()
{
	Vector3 katamariPos = katamari->position;
	float katamariLogicalRadius = katamari->currentRadius;
	float katamariVisualRadius = katamari->radius;

	for (auto* pickup : pickups)
	{
		if (collectedPickups.count(pickup) > 0)
			continue;

		float distance = Vector3::Distance(katamariPos, pickup->position);
		float visualCollisionDist = katamariVisualRadius + pickup->boundingRadius;

		if (distance < visualCollisionDist)
		{
			if (pickup->boundingRadius < katamariLogicalRadius)
			{
				katamari->absorb(pickup->boundingRadius);
				katamari->stickModel(pickup);
				collectedPickups.insert(pickup);
			}
			else
			{
				Vector3 pushDir = katamariPos - pickup->position;
				pushDir.y = 0.0f;
				if (pushDir.LengthSquared() > 0.001f)
				{
					pushDir.Normalize();
					float overlap = visualCollisionDist - distance;
					katamari->position += pushDir * overlap * 0.5f;
				}
			}
		}
	}
}
