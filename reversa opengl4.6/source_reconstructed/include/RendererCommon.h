#pragma once
#include <array>
#include <cstdint>
#include <vector>

namespace reverse_gl46 {

enum class EvidenceLevel : std::uint8_t { Extracted, Reconstructed, Inferred };
enum class RendererMode : int { LegacyGL2 = 0, OpenGL33Compatibility = 1, OpenGL46 = 2 };

struct Vec2 { float x{}, y{}; };
struct Vec3 { float x{}, y{}, z{}; };
struct Vec4 { float x{}, y{}, z{}, w{}; };
struct Mat4 { float m[16]{}; };

// Layout literal do UBO encontrado em vários shaders GL4 do binário.
struct FrameDataStd140 {
    Mat4 frameView;
    Mat4 frameProjection;
    Mat4 frameViewProjection;
    Vec4 frameCameraPosition;
    Vec4 frameViewport;
    Vec4 frameTimeFog;
    Vec4 frameTint;
};

struct ModelVertex {
    float position[3]{};
    float normal[3]{};
    float texcoord[2]{};
    std::uint32_t bone{};
};

static_assert(sizeof(ModelVertex) == 36, "layout reconstruido esperado");

} // namespace reverse_gl46
