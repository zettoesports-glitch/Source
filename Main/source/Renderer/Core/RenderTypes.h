#pragma once

#include <cstdint>

namespace MU::Render
{
    enum class RendererBackend : std::uint8_t
    {
        Legacy = 0,
        OpenGL = 1,
        Vulkan = 2
    };

    struct RenderViewport
    {
        int Width = 0;
        int Height = 0;
    };

    using ShaderHandle = std::uint32_t;
    using TextureHandle = std::uint32_t;
    using BufferHandle = std::uint32_t;
    using PipelineHandle = std::uint32_t;

    constexpr ShaderHandle InvalidShader = 0;
    constexpr TextureHandle InvalidTexture = 0;
    constexpr BufferHandle InvalidBuffer = 0;
    constexpr PipelineHandle InvalidPipeline = 0;
}
