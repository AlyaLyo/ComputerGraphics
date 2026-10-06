#pragma once

#include <algorithm>

#include "SimpleMath.h"
#include "InputDevice.h"
#include "GameComponent.h"
#include "BodyComponent.h"

class Game;

enum class ProjectionMode
{
	Perspective,
	Orthographic
};

enum class ViewMode
{
	FPS,
	Orbit,
	TP
};

struct ControlConfig {
	float orbitRotateSensitivity{ 0.005f };
	float freeRotateSensitivity{ 0.004f };
	float orbitZoomSensitivity{ 0.001f };
	float minOrbitRadius{ 2.0f };
	float maxOrbitRadius{ 100.0f };
	float pitchClamp{ DirectX::XM_PIDIV2 - 0.01f };
};

class Camera
{
public:

	Camera(Game* inGame);

	Game* game;
	BodyComponent* targetComponent = nullptr;
	std::vector<BodyComponent*> targets;
	size_t currentTargetIndex = 0;
	float aspectRatio;

	ControlConfig config;

	void setupTPCamera(BodyComponent* target, float distance, float heightOffset);

	void switchProjection();
	void switchView();

	void onMouseMove();
	void update();
	void updateMatrix();

	DirectX::SimpleMath::Matrix getMatrix() const;
	DirectX::SimpleMath::Vector3 GetPosition() const { return position; }
	DirectX::SimpleMath::Vector3 GetForward() const { return forward; }
	DirectX::SimpleMath::Vector3 GetRight() const { return right; }
	DirectX::SimpleMath::Vector3 GetLookAtPoint() const { return lookAtPoint; }

	float GetFOV() const { return fov; }
	float GetNearPlane() const { return nearPlaneDistance; }
	float GetFarPlane() const { return farPlaneDistance; }
	DirectX::SimpleMath::Matrix GetView() const { return viewMatrix; }
	DirectX::SimpleMath::Matrix GetProj() const { return projectionMatrix; }
	std::vector<DirectX::SimpleMath::Vector4> GetFrustumCornersWorldSpace(const DirectX::SimpleMath::Matrix& view, const DirectX::SimpleMath::Matrix& proj);

private:

	DirectX::SimpleMath::Matrix viewMatrix;
	DirectX::SimpleMath::Matrix projectionMatrix;

	DirectX::SimpleMath::Vector3 position;
	DirectX::SimpleMath::Vector3 relativePosition{ 0, 0, -20 };
	DirectX::SimpleMath::Vector3 lookAtPoint;

	DirectX::SimpleMath::Vector3 up;
	DirectX::SimpleMath::Vector3 right;
	DirectX::SimpleMath::Vector3 forward;

	float fov;
	float nearPlaneDistance;
	float farPlaneDistance;

	float movementSpeed;

	float yaw;
	float pitch;

	float tpYaw{ 0.0f };
	float tpPitch{ 0.0f };
	float followDistance{ 20.0f };
	float followHeightOffset{ 15.0f };

	ProjectionMode projectionMode;
	ViewMode viewMode;

	bool spaceKeyWasPressed = false;
	bool projectionKeyWasPressed = false;
	bool viewKeyWasPressed = false;

	void updateProjectionMatrix();
	void updateViewMatrix();

	void rotateOrbitCamera(float deltaX, float deltaY);
	void zoomOrbitCamera(float wheelDelta);

	void rotateFreeCamera(float deltaX, float deltaY);
	void rotateTPCamera(float deltaX, float deltaY);

	float normalizeAngle(float angle);
	float clampPitch(float pitch, float limit);
	DirectX::SimpleMath::Vector3 rotateVectorByQuaternion(const DirectX::SimpleMath::Vector3& vec, const DirectX::SimpleMath::Quaternion& q);

};