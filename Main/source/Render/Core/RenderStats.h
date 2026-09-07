#pragma once

#include <cstdint>

namespace Render
{
    struct RenderStats
    {
        uint64_t drawCalls;
        uint64_t indexedDrawCalls;
        uint64_t triangles;
        uint64_t shaderBinds;
        uint64_t textureBinds;
        uint64_t bufferUploads;
        uint64_t bufferUploadBytes;

        RenderStats() { Reset(); }

        void Reset()
        {
            drawCalls = 0;
            indexedDrawCalls = 0;
            triangles = 0;
            shaderBinds = 0;
            textureBinds = 0;
            bufferUploads = 0;
            bufferUploadBytes = 0;
        }
    };
}
