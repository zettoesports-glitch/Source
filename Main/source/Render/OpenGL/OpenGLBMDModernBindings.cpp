#include "stdafx.h"
#include "OpenGLBMDModernBindings.h"
#include "OpenGLSkeletonTexture.h"

OpenGLBMDModernBindings::OpenGLBMDModernBindings()
    : m_Program(0)
    , m_MaterialSamplerLocation(-1)
    , m_SkeletonSamplerLocation(-1)
    , m_Bound(false)
    , m_PreviousActiveTexture(GL_TEXTURE0)
    , m_PreviousMaterialTexture(0)
    , m_PreviousSkeletonTexture(0)
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
        skeletonTexture.GetTextureId() == 0 || m_Bound)
        return false;

    // Capture the exact legacy texture state before taking ownership of units
    // 0 and 1. The old implementation only remembered the active unit and then
    // unbound both textures to zero. That prevented modern-state leakage, but it
    // could also destroy bindings that the legacy renderer expected to survive.
    glGetIntegerv(GL_ACTIVE_TEXTURE, &m_PreviousActiveTexture);

    GLint previousBinding = 0;
    glActiveTexture(GL_TEXTURE0 + MaterialTextureUnit);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &previousBinding);
    m_PreviousMaterialTexture = static_cast<unsigned int>(previousBinding);
    if (m_MaterialSamplerLocation >= 0)
        glBindTexture(GL_TEXTURE_2D, materialTexture);

    glActiveTexture(GL_TEXTURE0 + SkeletonTextureUnit);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &previousBinding);
    m_PreviousSkeletonTexture = static_cast<unsigned int>(previousBinding);
    glBindTexture(GL_TEXTURE_2D, skeletonTexture.GetTextureId());

    glActiveTexture(static_cast<GLenum>(m_PreviousActiveTexture));
    m_Bound = true;
    return true;
}

void OpenGLBMDModernBindings::Unbind() const
{
    if (!m_Bound)
        return;

    // Restore rather than clear. This gives the modern pass strict ownership
    // of its temporary bindings without making assumptions about legacy state.
    glActiveTexture(GL_TEXTURE0 + SkeletonTextureUnit);
    glBindTexture(GL_TEXTURE_2D, m_PreviousSkeletonTexture);

    glActiveTexture(GL_TEXTURE0 + MaterialTextureUnit);
    glBindTexture(GL_TEXTURE_2D, m_PreviousMaterialTexture);

    glActiveTexture(static_cast<GLenum>(m_PreviousActiveTexture));

    m_PreviousActiveTexture = GL_TEXTURE0;
    m_PreviousMaterialTexture = 0;
    m_PreviousSkeletonTexture = 0;
    m_Bound = false;
}
