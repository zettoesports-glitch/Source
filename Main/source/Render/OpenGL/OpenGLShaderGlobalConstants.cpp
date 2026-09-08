#include "stdafx.h"
#include "OpenGLShaderGlobalConstants.h"

#include <cstring>

OpenGLShaderGlobalConstants::OpenGLShaderGlobalConstants()
    : m_Buffer(0)
    , m_HasLastConstants(false)
    , m_Bound(false)
    , m_PreviousBaseBuffer(0)
{
}

OpenGLShaderGlobalConstants::~OpenGLShaderGlobalConstants()
{
    Destroy();
}

bool OpenGLShaderGlobalConstants::Initialize()
{
    if (m_Buffer != 0)
        return true;

    glGenBuffers(1, &m_Buffer);
    if (m_Buffer == 0)
        return false;

    GLint previousUniformBuffer = 0;
    glGetIntegerv(GL_UNIFORM_BUFFER_BINDING, &previousUniformBuffer);

    glBindBuffer(GL_UNIFORM_BUFFER, m_Buffer);
    glBufferData(GL_UNIFORM_BUFFER,
                 sizeof(Render::ShaderGlobalConstants),
                 NULL,
                 GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, static_cast<GLuint>(previousUniformBuffer));
    return true;
}

bool OpenGLShaderGlobalConstants::ConfigureProgram(unsigned int program) const
{
    if (program == 0 || m_Buffer == 0)
        return false;

    const GLuint blockIndex = glGetUniformBlockIndex(program, "GlobalConstants");
    if (blockIndex == GL_INVALID_INDEX)
        return false;

    glUniformBlockBinding(program,
                          blockIndex,
                          Render::ShaderGlobalConstantsBinding);
    return true;
}

bool OpenGLShaderGlobalConstants::Upload(const Render::ShaderGlobalConstants& constants)
{
    if (!Initialize())
        return false;

    // Port of the useful part of Sven's GLP-10 idea: repeated meshes for the
    // same object normally share the exact same camera/light/time constants.
    // Avoid issuing another glBufferSubData when the 304-byte std140 payload is
    // byte-identical; binding-point ownership is handled separately by Bind().
    if (m_HasLastConstants &&
        std::memcmp(&m_LastConstants,
                    &constants,
                    sizeof(Render::ShaderGlobalConstants)) == 0)
    {
        return true;
    }

    GLint previousUniformBuffer = 0;
    glGetIntegerv(GL_UNIFORM_BUFFER_BINDING, &previousUniformBuffer);

    glBindBuffer(GL_UNIFORM_BUFFER, m_Buffer);
    glBufferSubData(GL_UNIFORM_BUFFER,
                    0,
                    sizeof(Render::ShaderGlobalConstants),
                    &constants);
    glBindBuffer(GL_UNIFORM_BUFFER, static_cast<GLuint>(previousUniformBuffer));

    m_LastConstants = constants;
    m_HasLastConstants = true;
    return true;
}

void OpenGLShaderGlobalConstants::Bind() const
{
    if (m_Buffer == 0 || m_Bound)
        return;

    GLint previousBaseBuffer = 0;
    glGetIntegeri_v(GL_UNIFORM_BUFFER_BINDING,
                    Render::ShaderGlobalConstantsBinding,
                    &previousBaseBuffer);
    m_PreviousBaseBuffer = static_cast<unsigned int>(previousBaseBuffer);

    glBindBufferBase(GL_UNIFORM_BUFFER,
                     Render::ShaderGlobalConstantsBinding,
                     m_Buffer);
    m_Bound = true;
}

void OpenGLShaderGlobalConstants::Unbind() const
{
    if (!m_Bound)
        return;

    // Restore the exact indexed UBO binding that existed before ModernBMD.
    // Clearing to zero can invalidate another Core-style renderer that shares
    // this binding point with the still-active legacy/world frame.
    glBindBufferBase(GL_UNIFORM_BUFFER,
                     Render::ShaderGlobalConstantsBinding,
                     m_PreviousBaseBuffer);
    m_PreviousBaseBuffer = 0;
    m_Bound = false;
}

void OpenGLShaderGlobalConstants::Destroy()
{
    Unbind();

    if (m_Buffer != 0)
    {
        glDeleteBuffers(1, &m_Buffer);
        m_Buffer = 0;
    }

    m_HasLastConstants = false;
}
