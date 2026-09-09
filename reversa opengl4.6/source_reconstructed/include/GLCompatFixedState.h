#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
namespace reverse_gl46 {
enum class LegacyCategory : std::uint8_t { ImmediateMode,MatrixStack,FixedCapability,FixedFog,TextureEnvironment,ClientArray,AttributeStack,RemovedQuery,QuadsDraw,CurrentColor,Count };
class LegacyReachability {
public:
    void Hit(LegacyCategory category) { ++hits_[static_cast<std::size_t>(category)]; }
    std::uint64_t Hits(LegacyCategory category) const { return hits_[static_cast<std::size_t>(category)]; }
private:
    std::array<std::uint64_t,static_cast<std::size_t>(LegacyCategory::Count)> hits_{};
};
}
