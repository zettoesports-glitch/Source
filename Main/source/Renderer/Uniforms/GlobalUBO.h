#pragma once

#include <glm/glm.hpp>

namespace MU::Render
{
    struct alignas(16) GlobalUBO
    {
        glm::mat4 View{1.0f};
        glm::mat4 Projection{1.0f};
        glm::mat4 ViewProjection{1.0f};
        glm::vec4 CameraPosition{0.0f};
        glm::vec4 ScreenSize{0.0f};
        glm::vec4 Time{0.0f};
    };

    static_assert(sizeof(GlobalUBO) % 16 == 0, "GlobalUBO must be std140-friendly.");
}
