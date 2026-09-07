#include "stdafx.h"
#include "OpenGLSkeletonTexture.h"
#include "../Model/SkeletonBuffer.h"

#include <algorithm>

OpenGLSkeletonTexture::OpenGLSkeletonTexture()
    : m_Texture(0)
    , m_Width(DefaultWidth)
    , m_Height(0)
    , m_UploadedGeneration(0)
{
}

OpenGLSkeletonTexture::~OpenGLSkeletonTexture()
{
    Destroy();
}

bool OpenGLSkeletonTexture::Upload(const SkeletonBuffer& skeleton)
{
    const std::vector<float>& texels = skeleton.GetTexels();
    const size_t texelCount = texels.size() / 4u;

    if (texels.empty() || texelCount == 0 || (texels.size() % 4u) != 0u)
        return false;

    const std::uint32_t requiredHeight =
        static_cast<std::uint32_t>((texelCount + m_Width - 1u) / m_Width);

    GLint maxTextureSize = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    if (maxTextureSize <= 0 ||
        m_Width > static_cast<std::uint32_t>(maxTextureSize) ||
        requiredHeight == 0 ||
        requiredHeight > static_cast<std::uint32_t>(maxTextureSize))
    {
        return false;
    }

    // Upload is allowed to run before a mixed modern/legacy draw loop. Preserve
    // the caller's active unit and GL_TEXTURE_2D binding so atlas preparation
    // cannot contaminate the following legacy command.
    GLint previousActiveTexture = GL_TEXTURE0;
    GLint previousTextureBinding = 0;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &previousActiveTexture);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &previousTextureBinding);

    if (m_Texture == 0)
    {
        glGenTextures(1, &m_Texture);
        if (m_Texture == 0)
            return false;

        glBindTexture(GL_TEXTURE_2D, m_Texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, m_Texture);
    }

    // texelFetch addresses a fixed 1024-wide texture. Pad the final row so the
    // upload always covers complete rows.
    std::vector<float> upload;
    const size_t requiredFloats = static_cast<size_t>(m_Width) * requiredHeight * 4u;
    if (texels.size() != requiredFloats)
    {
        upload.assign(requiredFloats, 0.0f);
        std::copy(texels.begin(), texels.end(), upload.begin());
    }

    const float* data = upload.empty() ? texels.data() : upload.data();

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    if (m_Height < requiredHeight)
    {
        // Grow only when necessary. Once allocated, subsequent batches that fit
        // use glTexSubImage2D instead of reallocating GPU storage every draw.
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA32F,
            static_cast<GLsizei>(m_Width),
            static_cast<GLsizei>(requiredHeight),
            0,
            GL_RGBA,
            GL_FLOAT,
            data);
        m_Height = requiredHeight;
    }
    else
    {
        glTexSubImage2D(
            GL_TEXTURE_2D,
            0,
            0,
            0,
            static_cast<GLsizei>(m_Width),
            static_cast<GLsizei>(requiredHeight),
            GL_RGBA,
            GL_FLOAT,
            data);
    }

    glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(previousTextureBinding));
    glActiveTexture(static_cast<GLenum>(previousActiveTexture));

    m_UploadedGeneration = skeleton.GetGeneration();
    return true;
}

void OpenGLSkeletonTexture::Bind(std::uint32_t textureUnit) const
{
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, m_Texture);
}

void OpenGLSkeletonTexture::Destroy()
{
    if (m_Texture != 0)
    {
        glDeleteTextures(1, &m_Texture);
        m_Texture = 0;
    }

    m_Height = 0;
    m_UploadedGeneration = 0;
}
