#pragma once

#include <cstdint>

namespace Render
{
    struct alignas(16) GlobalUBO
    {
        float view[16];
        float projection[16];
        float viewProjection[16];
        float cameraPosition[4];
        float worldTime[4];
        float screenSize[4];
    };

    static const uint32_t GlobalUBOSlot = 0;
}
