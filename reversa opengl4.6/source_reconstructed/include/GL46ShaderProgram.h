#pragma once
#include <cstdint>
#include <string>
#include <string_view>

namespace reverse_gl46 {

using GLUInt = std::uint32_t;
using GLEnum = std::uint32_t;
using GLInt = int;
using GLSizeI = int;

constexpr GLEnum kVertexShader = 0x8B31;
constexpr GLEnum kFragmentShader = 0x8B30;
constexpr GLEnum kCompileStatus = 0x8B81;
constexpr GLEnum kLinkStatus = 0x8B82;

struct ShaderSourceParts {
    const char* version{};
    const char* defineBlock{};
    const char* body{};
};

struct ShaderProgramApi {
    GLUInt (*createShader)(GLEnum){};
    void (*shaderSource)(GLUInt, GLSizeI, const char* const*, const GLInt*){};
    void (*compileShader)(GLUInt){};
    void (*getShaderiv)(GLUInt, GLEnum, GLInt*){};
    void (*getShaderInfoLog)(GLUInt, GLSizeI, GLSizeI*, char*){};
    void (*deleteShader)(GLUInt){};
    GLUInt (*createProgram)(){};
    void (*attachShader)(GLUInt, GLUInt){};
    void (*linkProgram)(GLUInt){};
    void (*getProgramiv)(GLUInt, GLEnum, GLInt*){};
    void (*getProgramInfoLog)(GLUInt, GLSizeI, GLSizeI*, char*){};
    void (*detachShader)(GLUInt, GLUInt){};
    void (*deleteProgram)(GLUInt){};
};

struct ProgramBuildResult {
    GLUInt program{};
    std::string vertexLog;
    std::string fragmentLog;
    std::string programLog;
};

// Clean-room equivalente à montagem observada em 0x0046F040:
// glShaderSource(shader, 3, {version, define, body}, nullptr).
bool BuildProgram3Part(const ShaderProgramApi& api,
                       const ShaderSourceParts& vertex,
                       const ShaderSourceParts& fragment,
                       ProgramBuildResult& out);

} // namespace reverse_gl46
