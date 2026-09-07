#pragma once

#include <cstdint>

namespace MU::Render
{
    struct RenderConfig
    {
        int Width = 0;
        int Height = 0;
        bool VSync = true;
        bool DebugValidation = false;
        std::uint32_t MaxBones = 200;
    };

    constexpr std::uint32_t DefaultMaxBones = 200;
}
