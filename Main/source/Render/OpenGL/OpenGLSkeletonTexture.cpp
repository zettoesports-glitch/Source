#include "stdafx.h"
#include "OpenGLSkeletonTexture.h"
#include "SkeletonBuffer.h"

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

    if (texels.empty() || texelCount == 0)
        return false;

    const std::uint32_t requiredHeight =
        static_cast<std::uint32_t>((texelCount + m_Width - 1u) / m_Width);

    // The shader uses texelFetch, therefore filtering and mipmaps are disabled.
    if (m_Texture == 0)
    {
        glGenTextures(1, &m_Texture);
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

    // Pad the last row so glTexImage2D always receives width * height texels.
    std::vector<float> upload;
    const size_t requiredFloats = static_cast<size_t>(m_Width) * requiredHeight * 4u;
    if (texels.size() != requiredFloats)
    {
        upload.assign(requiredFloats, 0.0f);
        std::copy(texels.begin(), texels.end(), upload.begin());
    }

    const float* data = upload.empty() ? texels.data() : upload.data();

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
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