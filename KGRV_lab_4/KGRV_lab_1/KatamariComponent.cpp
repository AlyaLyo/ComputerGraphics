#include "KatamariComponent.h"
#include "Game.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;
using namespace std;

KatamariComponent::KatamariComponent(Game* game, float initialRadius, Vector4 color) :
	BodyComponent(game, initialRadius, color),
	currentRadius(initialRadius)
{
	radius = initialRadius;
	currentRadius = initialRadius;
	savedRotation = Quaternion::Identity;
	velocity = Vector3::Zero;
	rotationSpeed = 0.1f;
	rotationDrag = 0.28f;
	movementSpeed = 5.0f;
	movementDrag = 10.0f;
}

KatamariComponent::~KatamariComponent()
{}


void KatamariComponent::update()
{
	float dt = game->deltaTime;
	
	Vector3 camForward = game->camera->GetForward();
	camForward.y = 0.0f;
	if (camForward.LengthSquared() > 0.001f)
		camForward.Normalize();
	else
		camForward = Vector3::Forward;

	Vector3 camRight = camForward.Cross(Vector3::Up);
	if (camRight.LengthSquared() > 0.001f)
		camRight.Normalize();
	else
		camRight = Vector3::Right;

	Vector3 moveDir(0.0f, 0.0f, 0.0f);
	if (game->inputDevice->IsKeyDown(Keys::W)) moveDir += camForward;
	if (game->inputDevice->IsKeyDown(Keys::S)) moveDir -= camForward;
	if (game->inputDevice->IsKeyDown(Keys::A)) moveDir -= camRight;
	if (game->inputDevice->IsKeyDown(Keys::D)) moveDir += camRight;

	if (moveDir.LengthSquared() > 0.001f)
	{
		moveDir.Normalize();
		setDirection(moveDir);
	}

	savedRotation.RotateTowards(Quaternion::Identity, rotationDrag * dt);

	rotation *= savedRotation;
	rotation.Normalize();

	velocity *= 1.0f - movementDrag * dt;

	position += velocity * dt;

	float sceneHalfSize = 48.0f;
	position.x = clamp(position.x, -sceneHalfSize, sceneHalfSize);
	position.z = clamp(position.z, -sceneHalfSize, sceneHalfSize);
	position.y = radius;

	for (auto& info : stuckModels)
	{

		Quaternion rotDelta = info.stickTimeBallRotationInverse * rotation;
		info.pickup->position = position + Vector3::Transform(info.localPosition, rotDelta);
		info.pickup->rotation = info.savedWorldRotation * rotDelta;

	}

	//this->scale = currentRadius / radius;
	BodyComponent::update();
}

void KatamariComponent::absorb(float objectRadius)
{
	float oldVolume = currentRadius * currentRadius * currentRadius;
	float objectVolume = objectRadius * objectRadius * objectRadius;
	float newVolume = oldVolume + objectVolume;
	currentRadius = powf(newVolume, 1.0f / 3.0f);
}

void KatamariComponent::stickModel(PickupComponent* pickup)
{
	if (!pickup)
	{
		return;
	}
	
	StuckModelInfo info;
	info.pickup = pickup;
	info.localPosition = pickup->position - position;
	info.stickTimeBallRotationInverse = Quaternion(-rotation.x, -rotation.y, -rotation.z, rotation.w);
	info.savedWorldRotation = pickup->rotation;

	stuckModels.push_back(info);
}

void KatamariComponent::setDirection(Vector3 dir)
{
	Vector3 tmp = Vector3(dir.x, 0.0f, dir.z);
	tmp.Normalize();

	Quaternion q = Quaternion::CreateFromAxisAngle(tmp.Cross(Vector3::Up), -rotationSpeed);
	float f = Quaternion::Angle(Quaternion::Identity, savedRotation) / 0.1f;
	savedRotation *= Quaternion::Lerp(q, Quaternion::Identity, f);

	velocity = tmp * movementSpeed;
}