#pragma once

#include <cstdint>

namespace MU::Render
{
    struct RenderStats
    {
        std::uint64_t DrawCalls = 0;
        std::uint64_t IndexedDrawCalls = 0;
        std::uint64_t Triangles = 0;
        std::uint64_t ShaderBinds = 0;
        std::uint64_t TextureBinds = 0;
        std::uint64_t BufferUpdates = 0;

        void Reset()
        {
            *this = RenderStats{};
        }
    };
}
