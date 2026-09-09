#pragma once
#include <cstdint>
namespace reverse_gl46 {
struct GL46Capabilities {
    bool profileCompatibility{};
    bool profileCore{};
    bool gl33Bridge{};
    bool gl46Bridge{};
    bool shaderObjects{};
    bool bufferObjects{};
    bool vertexArrayObjects{};
    bool textureStorage{};
    bool timerQuery{};
    bool debugOutput{};
    bool directStateAccess{};
    bool bufferStorage{};
    bool uniformBuffer{};
    bool shaderStorageBuffer{};
    bool samplerObjects{};
    bool syncObjects{};
    bool instancedDrawing{};
    bool multiDrawIndirect{};
    bool computeShader{};
    std::int32_t maxTextureSize{};
    std::int32_t maxTextureUnits{};
    std::int32_t maxVertexAttribs{};
    std::int32_t maxUniformBufferBindings{};
    std::int32_t maxShaderStorageBufferBindings{};
};
// Estrutura reconstruída a partir das chaves cap_* presentes no Main.exe.
GL46Capabilities QueryGL46Capabilities();
}
