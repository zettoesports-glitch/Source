#pragma once

#include <cstdint>

namespace Render
{
    struct alignas(16) SceneUBO
    {
        float fogColor[4];
        float fogParams[4];
        float sunDirection[4];
        float lightPosition[4];
        float ambientLight[4];
        float environment[4];
        float wind[4];
        float water[4];
    };

    static const uint32_t SceneUBOSlot = 1;
}
