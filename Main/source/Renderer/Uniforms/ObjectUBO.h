#pragma once

#include <glm/glm.hpp>

namespace MU::Render
{
    struct alignas(16) ObjectUBO
    {
        glm::mat4 Model{1.0f};
        glm::mat4 Normal{1.0f};
        glm::vec4 Parameters{0.0f};
    };

    static_assert(sizeof(ObjectUBO) % 16 == 0, "ObjectUBO must be std140-friendly.");
}
