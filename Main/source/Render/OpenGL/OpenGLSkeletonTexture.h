#pragma once

#include <cstdint>

class SkeletonBuffer;

// OpenGL 3.3+ backend for the shared skeleton frontend.
// Uses RGBA32F sampler2D because the reference GLSL declares sampler2D BonesTexture.
class OpenGLSkeletonTexture
{
public:
    static constexpr std::uint32_t DefaultWidth = 1024;

    OpenGLSkeletonTexture();
    ~OpenGLSkeletonTexture();

    OpenGLSkeletonTexture(const OpenGLSkeletonTexture&) = delete;
    OpenGLSkeletonTexture& operator=(const OpenGLSkeletonTexture&) = delete;

    bool Upload(const SkeletonBuffer& skeleton);
    void Bind(std::uint32_t textureUnit) const;
    void Destroy();

    unsigned int GetTextureId() const { return m_Texture; }
    std::uint32_t GetWidth() const { return m_Width; }
    std::uint32_t GetHeight() const { return m_Height; }
    std::uint64_t GetUploadedGeneration() const { return m_UploadedGeneration; }

private:
    unsigned int m_Texture;
    std::uint32_t m_Width;
    std::uint32_t m_Height;
    std::uint64_t m_UploadedGeneration;
};