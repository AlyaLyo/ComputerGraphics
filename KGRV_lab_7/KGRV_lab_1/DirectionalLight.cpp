#include "DirectionalLight.h"
#include "Game.h"

using namespace DirectX;
using namespace SimpleMath;

DirectionalLight::DirectionalLight(Game* g): 
    lightDirection_(0.0f, 1.0f, 0.0f, 0.0f),
    lightColor_(1.0f, 1.0f, 1.0f, 1.0f),
    game_(g)
{
    shadowCascadeLevels_.push_back(1000.0f / 50.0f);  // 20.0
    shadowCascadeLevels_.push_back(1000.0f / 25.0f);  // 40.0
    shadowCascadeLevels_.push_back(1000.0f / 10.0f);  // 100.0
    shadowCascadeLevels_.push_back(1000.0f / 2.0f);   // 500.0
}

Matrix DirectionalLight::GetLightSpaceMatrix(float nearPlane, float farPlane)
{
    const auto proj = Matrix::CreatePerspectiveFieldOfView(
        game_->camera->GetFOV(), game_->camera->aspectRatio, nearPlane, farPlane);
    const auto corners = game_->camera->GetFrustumCornersWorldSpace(game_->camera->GetView(), proj);

    auto center = Vector3::Zero;
    for (const auto& v : corners)
    {
        center.x += v.x;
        center.y += v.y;
        center.z += v.z;
    }
    center /= static_cast<float>(corners.size());

    const auto lightView = Matrix::CreateLookAt(center, center - Vector3(lightDirection_.x, lightDirection_.y, lightDirection_.z), Vector3::Up);

    float minX = (std::numeric_limits<float>::max)();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = (std::numeric_limits<float>::max)();
    float maxY = std::numeric_limits<float>::lowest();
    float minZ = (std::numeric_limits<float>::max)();
    float maxZ = std::numeric_limits<float>::lowest();
    for (const auto& v : corners)
    {
        const auto trf = Vector4::Transform(v, lightView);
        minX = (std::min)(minX, trf.x);
        maxX = (std::max)(maxX, trf.x);
        minY = (std::min)(minY, trf.y);
        maxY = (std::max)(maxY, trf.y);
        minZ = (std::min)(minZ, trf.z);
        maxZ = (std::max)(maxZ, trf.z);
    }

    constexpr float zMult = 10.0f;
    minZ = (minZ < 0) ? minZ * zMult : minZ / zMult;
    maxZ = (maxZ < 0) ? maxZ / zMult : maxZ * zMult;

    const auto lightProjection = Matrix::CreateOrthographicOffCenter(minX, maxX, minY, maxY, minZ, maxZ);

    return lightView * lightProjection;
}

Vector4 DirectionalLight::GetShadowCascadeDistances() const
{
    return Vector4(shadowCascadeLevels_[0], shadowCascadeLevels_[1], shadowCascadeLevels_[2], shadowCascadeLevels_[3]);
}

std::vector<Matrix> DirectionalLight::GetLightSpaceMatrices()
{
    std::vector<Matrix> ret;
    for (size_t i = 0; i < shadowCascadeLevels_.size() + 1; ++i)
    {
        if (i == 0)
        {
            ret.push_back(GetLightSpaceMatrix(game_->camera->GetNearPlane(), shadowCascadeLevels_[i]));
        }
        else if (i < shadowCascadeLevels_.size())
        {
            ret.push_back(GetLightSpaceMatrix(shadowCascadeLevels_[i - 1], shadowCascadeLevels_[i]));
        }
        else
        {
            ret.push_back(GetLightSpaceMatrix(shadowCascadeLevels_[i - 1], game_->camera->GetFarPlane()));
        }
    }
    return ret;
}