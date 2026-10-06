#include "Camera.h"
#include "Game.h"

using namespace DirectX::SimpleMath;
using namespace DirectX;
using namespace std;

Camera::Camera(Game* inGame) :
	game(inGame)
{
	projectionMode = ProjectionMode::Perspective;
	viewMode = ViewMode::FPS;
	movementSpeed = 5.0f;
	followDistance = 25.0f;

	position = Vector3::Backward * 10.0f;
	lookAtPoint = Vector3::Zero;
	up = Vector3::Up;

	forward = Vector3::Forward;
	relativePosition = forward * 20.0f;
	yaw = 0.0f;
	pitch = 0.0f;

	fov = XM_PIDIV4;
	aspectRatio = 1.0f;
	nearPlaneDistance = 0.1f;
	farPlaneDistance = 1000.0f;
	
	viewMatrix = Matrix::CreateLookAt(position, lookAtPoint, up);
	projectionMatrix = Matrix::CreatePerspectiveFieldOfView(fov, aspectRatio, nearPlaneDistance, farPlaneDistance);
}

void Camera::setupTPCamera(BodyComponent* target, float distance, float heightOffset)
{
	viewMode = ViewMode::TP;
	targetComponent = target;
	followDistance = distance;
	followHeightOffset = heightOffset;
	tpYaw = 0.0f;

	if (target != nullptr)
	{
		Vector3 targetPos = target->getPosition();
		lookAtPoint = targetPos;

		position.x = targetPos.x - sinf(tpYaw) * followDistance;
		position.y = targetPos.y + followHeightOffset;
		position.z = targetPos.z - cosf(tpYaw) * followDistance;

		forward = lookAtPoint - position;
		forward.Normalize();

		right = Vector3::Up.Cross(forward);
		right.y = 0.0f;
		if (right.LengthSquared() > 0.001f)
			right.Normalize();
		else
			right = Vector3::Right;
	}
	else
	{
		lookAtPoint = Vector3::Zero;
		position = Vector3(0.0f, followHeightOffset, -followDistance);
		forward = Vector3::Forward;
		right = Vector3::Right;
	}

	up = Vector3::Up;
	updateMatrix();
}

void Camera::switchProjection()
{
	if (projectionMode == ProjectionMode::Perspective)
	{
		projectionMode = ProjectionMode::Orthographic;
		std::cout << "[CAMERA] Projection switched to: Orthographic" << std::endl;
	}
	else
	{
		projectionMode = ProjectionMode::Perspective;
		std::cout << "[CAMERA] Projection switched to: Perspective" << std::endl;
	}
}

void Camera::switchView()
{
	if (viewMode == ViewMode::Orbit)
	{
		viewMode = ViewMode::TP;
		std::cout << "[CAMERA] View switched to: TP" << std::endl;

		if (targetComponent != nullptr)
		{
			float horzDist = sqrtf(relativePosition.x * relativePosition.x + relativePosition.z * relativePosition.z);
			followDistance = (horzDist > 0.001f) ? horzDist : followDistance;
			followHeightOffset = relativePosition.y;

			if (horzDist > 0.001f)
				tpYaw = atan2f(-relativePosition.x, -relativePosition.z);
			else
				tpYaw = 0.0f;

			Vector3 targetPos = targetComponent->getPosition();
			lookAtPoint = targetPos;

			position.x = targetPos.x - sinf(tpYaw) * followDistance;
			position.y = targetPos.y + followHeightOffset;
			position.z = targetPos.z - cosf(tpYaw) * followDistance;

			forward = lookAtPoint - position;
			forward.Normalize();

			right = Vector3::Up.Cross(forward);
			right.y = 0.0f;
			if (right.LengthSquared() > 0.001f)
				right.Normalize();
			else
				right = Vector3::Right;

			up = Vector3::Up;
		}
	}
	else if (viewMode == ViewMode::TP)
	{
		viewMode = ViewMode::FPS;
		std::cout << "[CAMERA] View switched to: FPS" << std::endl;

		Vector3 target = (targetComponent != nullptr) ? targetComponent->getPosition() : Vector3::Zero;
		lookAtPoint = target;

		Vector3 dir = lookAtPoint - position;
		if (dir.LengthSquared() < 0.0001f)
		{
			dir = Vector3::Forward;
		}
		dir.Normalize();

		forward = dir;

		yaw = atan2f(forward.x, forward.z);
		pitch = asinf(forward.y);

		Matrix testOrientation = Matrix::CreateFromYawPitchRoll(yaw, pitch, 0.0f);
		Vector3 testForward = Vector3::Transform(Vector3::Forward, testOrientation);

		if (forward.Dot(testForward) < 0.0f)
		{
			yaw += 3.1415926535f; //pi
		}

		right = Vector3::Up.Cross(forward);
		right.Normalize();

		up = forward.Cross(right);
		up.Normalize();

		position -= forward * 2.0f;
	}
	else
	{
		viewMode = ViewMode::Orbit;
		relativePosition = position - lookAtPoint;
		std::cout << "[CAMERA] View switched to: Orbit" << std::endl;
	}
}

void Camera::onMouseMove()
{
	if (game->inputDevice->MouseOffset.LengthSquared() < 0.001f &&
		game->inputDevice->MouseWheelDelta == 0) 
	{
		return;
	}

	float deltaX = static_cast<float>(game->inputDevice->MouseOffset.x);
	float deltaY = static_cast<float>(game->inputDevice->MouseOffset.y);
	float wheelDelta = static_cast<float>(game->inputDevice->MouseWheelDelta);

	game->inputDevice->MouseOffset = Vector2::Zero;
	game->inputDevice->MouseWheelDelta = 0;

	bool isLMBPressed = game->inputDevice->IsKeyDown(Keys::LeftButton);

	if (viewMode == ViewMode::Orbit)
	{
		if (wheelDelta != 0) 
		{
			zoomOrbitCamera(wheelDelta);
		}
	}
	else if (viewMode == ViewMode::TP)
	{
		rotateTPCamera(deltaX);
	}
	else
	{
		if (isLMBPressed)
		{
			rotateFreeCamera(deltaX, deltaY);
		}
	}
}

void Camera::update()
{
	bool isProjectionPressed = game->inputDevice->IsKeyDown(Keys::E);
	if (isProjectionPressed && !projectionKeyWasPressed)
	{
		switchProjection();
	}
	projectionKeyWasPressed = isProjectionPressed;
	
	bool isViewPressed = game->inputDevice->IsKeyDown(Keys::Q);
	if (isViewPressed && !viewKeyWasPressed)
	{
		switchView();
	}
	viewKeyWasPressed = isViewPressed;
	
	if (viewMode == ViewMode::Orbit)
	{
		bool isSpacePressed = game->inputDevice->IsKeyDown(Keys::Space);
		if (isSpacePressed && !spaceKeyWasPressed)
		{

			targets.clear();
			for (auto* component : game->components)
			{
				if (auto* body = dynamic_cast<BodyComponent*>(component))
				{
					targets.push_back(dynamic_cast<BodyComponent*>(component));
				}
			}

			currentTargetIndex = (currentTargetIndex + 1) % targets.size();
			targetComponent = targets[currentTargetIndex];

		}
		spaceKeyWasPressed = isSpacePressed;

		if (targetComponent != nullptr)
		{
			Vector3 targetPosition = targetComponent->getPosition();
			lookAtPoint = targetPosition;
			position = targetPosition + relativePosition;
		}
		else
		{
			lookAtPoint = Vector3::Zero;
			position = Vector3::Backward * 10.0f;
		}
	}
	else if (viewMode == ViewMode::TP)
	{
		bool isSpacePressed = game->inputDevice->IsKeyDown(Keys::Space);
		if (isSpacePressed && !spaceKeyWasPressed)
		{
			targets.clear();
			for (auto* component : game->components)
			{
				if (auto* body = dynamic_cast<BodyComponent*>(component))
				{
					targets.push_back(dynamic_cast<BodyComponent*>(component));
				}
			}

			if (!targets.empty())
			{
				currentTargetIndex = (currentTargetIndex + 1) % targets.size();
				targetComponent = targets[currentTargetIndex];
			}
		}
		spaceKeyWasPressed = isSpacePressed;

		if (targetComponent != nullptr)
		{
			Vector3 targetPosition = targetComponent->getPosition();
			lookAtPoint = targetPosition;

			position.x = targetPosition.x - sinf(tpYaw) * followDistance;
			position.y = targetPosition.y + followHeightOffset;
			position.z = targetPosition.z - cosf(tpYaw) * followDistance;

			forward = lookAtPoint - position;
			forward.Normalize();

			right = Vector3::Up.Cross(forward);
			right.y = 0.0f;
			if (right.LengthSquared() > 0.001f)
				right.Normalize();
			else
				right = Vector3::Right;

			up = Vector3::Up;
		}
		else
		{
			lookAtPoint = Vector3::Zero;
			position = Vector3(0.0f, followHeightOffset, -followDistance);
		}
	}
	else
	{
		Matrix orientation = Matrix::CreateFromYawPitchRoll(yaw, pitch, 0.0f);

		forward = Vector3::Transform(Vector3::Forward, orientation);
		right = Vector3::Transform(Vector3::Right, orientation);
		up = Vector3::Transform(Vector3::Up, orientation);

		Vector3 movementDirection(0.0f, 0.0f, 0.0f);
		if (game->inputDevice->IsKeyDown(Keys::W)) movementDirection += forward;
		if (game->inputDevice->IsKeyDown(Keys::S)) movementDirection -= forward;
		if (game->inputDevice->IsKeyDown(Keys::A)) movementDirection -= right;
		if (game->inputDevice->IsKeyDown(Keys::D)) movementDirection += right;

		if (movementDirection.LengthSquared() > 0.001f)
		{
			movementDirection.Normalize();
			position += movementDirection * movementSpeed * game->deltaTime;
		}

		lookAtPoint = position + forward;
	}
	/*
	if (viewMode == ViewMode::FPS) {
		std::cout << "FPS Pos: " << position.x << "," << position.y << "," << position.z
			<< " | Dir: " << forward.x << "," << forward.y << "," << forward.z << "\n";
	}
	*/
	updateMatrix();
}


Matrix Camera::getMatrix() const
{
	return DirectX::SimpleMath::operator*(viewMatrix, projectionMatrix);
	//return viewMatrix * projectionMatrix;
}

void Camera::updateMatrix()
{
	updateViewMatrix();
	updateProjectionMatrix();
}

void Camera::updateViewMatrix()
{
	if (viewMode == ViewMode::Orbit)
	{
		viewMatrix = Matrix::CreateLookAt(position, lookAtPoint, up);
	}
	else if (viewMode == ViewMode::TP)
	{
		viewMatrix = Matrix::CreateLookAt(position, lookAtPoint, up);
	}
	else
	{
		viewMatrix = Matrix::CreateLookAt(position, position + forward, up);
	}
}

void Camera::updateProjectionMatrix()
{
	if (projectionMode == ProjectionMode::Perspective)
	{
		projectionMatrix = Matrix::CreatePerspectiveFieldOfView(fov, aspectRatio, nearPlaneDistance, farPlaneDistance);
	}
	else
	{
		float width = 50.0f;
		float height = width / aspectRatio;
		projectionMatrix = Matrix::CreateOrthographic(width, height, nearPlaneDistance, farPlaneDistance);
	}
}

void Camera::rotateOrbitCamera(float deltaX, float deltaY) {

	Vector3 dir = relativePosition;
	dir.Normalize();
	right = dir.Cross(up);
	right.Normalize();

	Quaternion yawRot = Quaternion::CreateFromAxisAngle(up, config.orbitRotateSensitivity * deltaX);
	Quaternion pitchRot = Quaternion::CreateFromAxisAngle(right, -config.orbitRotateSensitivity * deltaY);
	Quaternion combined = yawRot * pitchRot;

	relativePosition = rotateVectorByQuaternion(position, combined);
	up = rotateVectorByQuaternion(up, combined);

	Vector3 lookDir = relativePosition;
	lookDir.Normalize();
	up = up - lookDir * up.Dot(lookDir);
	up.Normalize();
}

void Camera::zoomOrbitCamera(float wheelDelta) {
	float zoomFactor = 1.0f - config.orbitZoomSensitivity * wheelDelta;
	relativePosition *= zoomFactor;

	float radius = relativePosition.Length();
	radius = std::clamp(radius, config.minOrbitRadius, config.maxOrbitRadius);

	if (radius > 0.0001f)
	{
		relativePosition.Normalize();
		relativePosition *= radius;
	}
}

void Camera::rotateFreeCamera(float deltaX, float deltaY) {
	yaw -= config.freeRotateSensitivity * deltaX;
	pitch -= config.freeRotateSensitivity * deltaY;

	yaw = normalizeAngle(yaw);
	pitch = clampPitch(pitch, config.pitchClamp);

	Matrix orientation = Matrix::CreateFromYawPitchRoll(yaw, pitch, 0.0f);

	up = Vector3::Transform(Vector3::Up, orientation);
	lookAtPoint = position + Vector3::Transform(Vector3::Forward, orientation);
}

void Camera::rotateTPCamera(float deltaX) {
	tpYaw -= config.freeRotateSensitivity * deltaX;
	tpYaw = normalizeAngle(tpYaw);
}

float Camera::normalizeAngle(float angle) {
	constexpr float TWO_PI = DirectX::XM_2PI;
	while (angle < -TWO_PI)
	{
		angle += TWO_PI;
	}
	while (angle > TWO_PI)
	{
		angle -= TWO_PI;
	}
	return angle;
}

float Camera::clampPitch(float pitch, float limit) {
	return std::clamp(pitch, -limit, limit);
}

Vector3 Camera::rotateVectorByQuaternion(const Vector3& vector, const Quaternion& q) {
	return Vector3::Transform(vector, q);
}

std::vector<Vector4> Camera::GetFrustumCornersWorldSpace(const Matrix& view, const Matrix& proj)
{
	const auto viewProj = view * proj;
	const auto inv = viewProj.Invert();

	std::vector<Vector4> frustumCorners;
	frustumCorners.reserve(8);
	for (unsigned int x = 0; x < 2; ++x)
	{
		for (unsigned int y = 0; y < 2; ++y)
		{
			for (unsigned int z = 0; z < 2; ++z)
			{
				const Vector4 pt = Vector4::Transform(
					Vector4(2.0f * static_cast<float>(x) - 1.0f,
						2.0f * static_cast<float>(y) - 1.0f,
						static_cast<float>(z), 1.0f), inv);
				frustumCorners.push_back(pt / pt.w);
			}
		}
	}

	return frustumCorners;
}