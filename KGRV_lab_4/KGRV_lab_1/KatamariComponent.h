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
#include "GameComponent.h"
#include "BodyComponent.h"
#include "PickupComponent.h"

class Game;

struct StuckModelInfo
{
    PickupComponent* pickup;
    DirectX::SimpleMath::Vector3 localPosition;
    DirectX::SimpleMath::Quaternion stickTimeBallRotationInverse;
    DirectX::SimpleMath::Quaternion savedWorldRotation;
};

class KatamariComponent :
	public BodyComponent
{
public:

	KatamariComponent(Game* game, float initialRadius, DirectX::SimpleMath::Vector4 color);
	~KatamariComponent();

    float radius;
    float currentRadius;

    void absorb(float objectRadius);
    void stickModel(PickupComponent* pickup);
    void setDirection(DirectX::SimpleMath::Vector3 dir);
    void update() override;

private:

    std::vector<StuckModelInfo> stuckModels;
    static constexpr float BASE_RADIUS = 1.0f;

    DirectX::SimpleMath::Quaternion savedRotation;
    DirectX::SimpleMath::Vector3 velocity;
    float rotationSpeed;
    float rotationDrag;
    float movementSpeed;
    float movementDrag;

};
