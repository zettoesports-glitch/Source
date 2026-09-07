#pragma once

#include "Renderer/Core/RenderConfig.h"

#include <array>
#include <glm/glm.hpp>

namespace MU::Render
{
    struct alignas(16) BoneUBO
    {
        std::array<glm::mat4, DefaultMaxBones> Matrices{};
    };

    static_assert(sizeof(BoneUBO) == DefaultMaxBones * sizeof(glm::mat4),
                  "BoneUBO layout must contain the complete 200-bone palette.");
    static_assert(sizeof(BoneUBO) % 16 == 0, "BoneUBO must be std140-friendly.");
}
