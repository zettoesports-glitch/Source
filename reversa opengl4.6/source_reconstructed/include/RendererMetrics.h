#pragma once
#include <cstdint>
namespace reverse_gl46 {
struct RendererMetricsSample {
    std::uint64_t frame{};
    double cpuFrameMs{};
    double gpuFrameMs{};
    std::uint32_t drawCalls{};
    std::uint64_t triangles{};
    std::uint32_t textureChanges{};
    std::uint32_t modelDraws{};
    std::uint32_t boneUploads{};
    std::uint64_t boneBytes{};
};
}
