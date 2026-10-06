#pragma once

#include "SimpleMath.h"

struct PerSceneCb
{
	DirectX::SimpleMath::Vector4 LightPos;
	DirectX::SimpleMath::Vector4 LightColor;
	DirectX::SimpleMath::Vector4 AmbientSpecularPowType;
	DirectX::SimpleMath::Matrix T;
	DirectX::SimpleMath::Vector4 CameraPos;
};