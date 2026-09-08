#include "stdafx.h"
#include "OpenGLShaderGlobalConstants.h"

#include <cstring>

OpenGLShaderGlobalConstants::OpenGLShaderGlobalConstants()
    : m_Buffer(0)
    , m_HasLastConstants(false)
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

    glBindBuffer(GL_UNIFORM_BUFFER, m_Buffer);
    glBufferData(GL_UNIFORM_BUFFER,
                 sizeof(Render::ShaderGlobalConstants),
                 NULL,
                 GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER,
                     Render::ShaderGlobalConstantsBinding,
                     m_Buffer);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
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
    // Keep the buffer bound to its expected slot, but avoid issuing another
    // glBufferSubData when the 304-byte std140 payload is byte-identical.
    if (m_HasLastConstants &&
        std::memcmp(&m_LastConstants,
                    &constants,
                    sizeof(Render::ShaderGlobalConstants)) == 0)
    {
        glBindBufferBase(GL_UNIFORM_BUFFER,
                         Render::ShaderGlobalConstantsBinding,
                         m_Buffer);
        return true;
    }

    glBindBuffer(GL_UNIFORM_BUFFER, m_Buffer);
    glBufferSubData(GL_UNIFORM_BUFFER,
                    0,
                    sizeof(Render::ShaderGlobalConstants),
                    &constants);
    glBindBufferBase(GL_UNIFORM_BUFFER,
                     Render::ShaderGlobalConstantsBinding,
                     m_Buffer);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    m_LastConstants = constants;
    m_HasLastConstants = true;
    return true;
}

void OpenGLShaderGlobalConstants::Bind() const
{
    if (m_Buffer != 0)
    {
        glBindBufferBase(GL_UNIFORM_BUFFER,
                         Render::ShaderGlobalConstantsBinding,
                         m_Buffer);
    }
}

void OpenGLShaderGlobalConstants::Unbind() const
{
    glBindBufferBase(GL_UNIFORM_BUFFER,
                     Render::ShaderGlobalConstantsBinding,
                     0);
}

void OpenGLShaderGlobalConstants::Destroy()
{
    if (m_Buffer != 0)
    {
        glDeleteBuffers(1, &m_Buffer);
        m_Buffer = 0;
    }

    m_HasLastConstants = false;
}
