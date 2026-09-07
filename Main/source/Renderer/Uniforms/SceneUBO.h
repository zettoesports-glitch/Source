#pragma once

#include <glm/glm.hpp>

namespace MU::Render
{
    struct alignas(16) SceneUBO
    {
        glm::vec4 FogColor{0.0f};
        glm::vec4 FogRange{0.0f};
        glm::vec4 LightPosition{0.0f};
        glm::vec4 AmbientLight{0.0f};
        glm::vec4 Wind{0.0f};
        glm::vec4 Water{0.0f};
        glm::vec4 ShadowColor{0.0f};
        glm::vec4 ShadowPosition{0.0f};
    };

    static_assert(sizeof(SceneUBO) % 16 == 0, "SceneUBO must be std140-friendly.");
}
