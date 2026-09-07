#pragma once

#include <cstdint>

namespace Render
{
    struct RenderConfig
    {
        bool enabled;
        bool useOpenGLDevice;
        bool useLegacyFallback;
        bool enableStats;
        uint32_t maxBones;

        RenderConfig()
            : enabled(true),
              useOpenGLDevice(true),
              useLegacyFallback(true),
              enableStats(true),
              maxBones(200) {}
    };

    inline const RenderConfig& GetDefaultRenderConfig()
    {
        static RenderConfig config;
        return config;
    }
}
