#pragma once

#include <cstddef>
#include <cstdint>

namespace Render
{
    // Exact CPU mirror of Shaders/common/GlobalConstantBuffers.inc and the
    // generated GLSL std140 GlobalConstants block. Keep this separate from the
    // generic RHI GlobalUBO until the frontend migration is complete.
    struct alignas(16) ShaderGlobalConstants
    {
        float ShadowColor[4];          //   0
        float ShadowPosition[4];       //  16
        float Shadow[16];              //  32
        float Game3D[16];              //  96
        float Game2D[16];              // 160
        float BillboardQ[4];           // 224
        float LightPosition[3];        // 240
        float WorldTime;               // 252
        float ChromeWave[3];           // 256
        float Random;                  // 268
        float Wave[2];                 // 272
        float WaterMove;               // 280
        float WindScale;               // 284
        float WindSpeed;               // 288
        std::int32_t WaterIndex;        // 292
        float Padding1[2];              // 296
    };

    static constexpr std::uint32_t ShaderGlobalConstantsBinding = 0;

    static_assert(offsetof(ShaderGlobalConstants, ShadowColor) == 0, "GlobalConstants layout mismatch");
    static_assert(offsetof(ShaderGlobalConstants, ShadowPosition) == 16, "GlobalConstants layout mismatch");
    static_assert(offsetof(ShaderGlobalConstants, Shadow) == 32, "GlobalConstants layout mismatch");
    static_assert(offsetof(ShaderGlobalConstants, Game3D) == 96, "GlobalConstants layout mismatch");
    static_assert(offsetof(ShaderGlobalConstants, Game2D) == 160, "GlobalConstants layout mismatch");
    static_assert(offsetof(ShaderGlobalConstants, BillboardQ) == 224, "GlobalConstants layout mismatch");
    static_assert(offsetof(ShaderGlobalConstants, LightPosition) == 240, "GlobalConstants layout mismatch");
    static_assert(offsetof(ShaderGlobalConstants, WorldTime) == 252, "GlobalConstants layout mismatch");
    static_assert(offsetof(ShaderGlobalConstants, ChromeWave) == 256, "GlobalConstants layout mismatch");
    static_assert(offsetof(ShaderGlobalConstants, Random) == 268, "GlobalConstants layout mismatch");
    static_assert(offsetof(ShaderGlobalConstants, Wave) == 272, "GlobalConstants layout mismatch");
    static_assert(offsetof(ShaderGlobalConstants, WaterMove) == 280, "GlobalConstants layout mismatch");
    static_assert(offsetof(ShaderGlobalConstants, WindScale) == 284, "GlobalConstants layout mismatch");
    static_assert(offsetof(ShaderGlobalConstants, WindSpeed) == 288, "GlobalConstants layout mismatch");
    static_assert(offsetof(ShaderGlobalConstants, WaterIndex) == 292, "GlobalConstants layout mismatch");
    static_assert(offsetof(ShaderGlobalConstants, Padding1) == 296, "GlobalConstants layout mismatch");
    static_assert(sizeof(ShaderGlobalConstants) == 304, "GlobalConstants size mismatch");
}
