#include "stdafx.h"
#include "OpenGLBMDModernBindings.h"
#include "OpenGLSkeletonTexture.h"

OpenGLBMDModernBindings::OpenGLBMDModernBindings()
    : m_Program(0)
    , m_MaterialSamplerLocation(-1)
    , m_SkeletonSamplerLocation(-1)
{
}

bool OpenGLBMDModernBindings::ConfigureProgram(unsigned int program)
{
    if (program == 0)
        return false;

    const GLint skeletonLocation = glGetUniformLocation(program, "BonesTexture");
    if (skeletonLocation < 0)
        return false;

    const GLint materialLocation = glGetUniformLocation(program, "Tex1");

    GLint previousProgram = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &previousProgram);
    if (static_cast<unsigned int>(previousProgram) != program)
        glUseProgram(program);

    // HLSL uses t0 independently per shader stage, while linked GLSL samplers
    // share the OpenGL texture-unit namespace. Keep them explicitly separate.
    if (materialLocation >= 0)
        glUniform1i(materialLocation, MaterialTextureUnit);
    glUniform1i(skeletonLocation, SkeletonTextureUnit);

    if (static_cast<unsigned int>(previousProgram) != program)
        glUseProgram(static_cast<GLuint>(previousProgram));

    m_Program = program;
    m_MaterialSamplerLocation = materialLocation;
    m_SkeletonSamplerLocation = skeletonLocation;
    return true;
}

bool OpenGLBMDModernBindings::Bind(
    unsigned int materialTexture,
    const OpenGLSkeletonTexture& skeletonTexture) const
{
    if (m_Program == 0 || m_SkeletonSamplerLocation < 0 ||
        skeletonTexture.GetTextureId() == 0)
        return false;

    GLint previousActiveTexture = GL_TEXTURE0;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &previousActiveTexture);

    if (m_MaterialSamplerLocation >= 0)
    {
        glActiveTexture(GL_TEXTURE0 + MaterialTextureUnit);
        glBindTexture(GL_TEXTURE_2D, materialTexture);
    }

    skeletonTexture.Bind(SkeletonTextureUnit);
    glActiveTexture(static_cast<GLenum>(previousActiveTexture));
    return true;
}

void OpenGLBMDModernBindings::Unbind() const
{
    GLint previousActiveTexture = GL_TEXTURE0;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &previousActiveTexture);
    glActiveTexture(GL_TEXTURE0 + SkeletonTextureUnit);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(static_cast<GLenum>(previousActiveTexture));
}
