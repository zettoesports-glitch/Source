#pragma once
#include "RendererCommon.h"
#include <array>
#include <cstdint>
namespace reverse_gl46 {
struct TerrainWeatherState {
    float rainIntensity{};
    float waterLevel{};
    float waterDepthRange{};
    float waterTurbidity{};
    float waterFrost{};
    float waterLava{};
    float weatherQuality{};
};
struct FootstepMark { Vec3 position{}; float radius{}; float age{}; float strength{}; };
struct TerrainFrameState {
    FrameDataStd140 frame{};
    TerrainWeatherState weather{};
    // Strings do shader indicam footprints/footsteps; capacidade observada: 8.
    std::array<FootstepMark,8> footsteps{};
    std::uint32_t footstepCount{};
};
class RendererTerrainGL46Reference {
public:
    void BeginFrame(const TerrainFrameState& state) { state_ = state; }
    const TerrainFrameState& State() const { return state_; }
private: TerrainFrameState state_{};
};
}
