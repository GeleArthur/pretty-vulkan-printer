#pragma once
#include <glm/mat4x4.hpp>

namespace pvp
{
    struct ModelCameraViewData
    {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };
} // namespace pvp