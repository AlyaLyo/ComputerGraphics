#pragma once

#include "SimpleMath.h"
#include <vector>
#include "PerSceneCb.h"

class Game;

class DirectionalLight
{
	DirectX::SimpleMath::Vector4 lightDirection_;
	DirectX::SimpleMath::Vector4 lightColor_;
	Game* game_;
	std::vector<float> shadowCascadeLevels_{};

public:
	DirectionalLight(Game* g);
	DirectX::SimpleMath::Vector4 GetDirection() const { return lightDirection_; }
	void SetDirection(const DirectX::SimpleMath::Vector4 dir) { lightDirection_ = dir; }
	DirectX::SimpleMath::Vector4 GetColor() const { return lightColor_; }
	void SetColor(const DirectX::SimpleMath::Vector4 col) { lightColor_ = col; }
	DirectX::SimpleMath::Matrix GetLightSpaceMatrix(float nearPlane, float farPlane);
	DirectX::SimpleMath::Vector4 GetShadowCascadeDistances() const;
	std::vector<DirectX::SimpleMath::Matrix> GetLightSpaceMatrices();
};
