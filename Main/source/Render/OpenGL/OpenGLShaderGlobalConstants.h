#pragma once

#include "../Uniforms/ShaderGlobalConstants.h"

class OpenGLShaderGlobalConstants
{
public:
    OpenGLShaderGlobalConstants();
    ~OpenGLShaderGlobalConstants();

    OpenGLShaderGlobalConstants(const OpenGLShaderGlobalConstants&) = delete;
    OpenGLShaderGlobalConstants& operator=(const OpenGLShaderGlobalConstants&) = delete;

    bool Initialize();
    bool ConfigureProgram(unsigned int program) const;
    bool Upload(const Render::ShaderGlobalConstants& constants);
    void Bind() const;
    void Unbind() const;
    void Destroy();

    unsigned int GetBuffer() const { return m_Buffer; }

private:
    unsigned int m_Buffer;
    Render::ShaderGlobalConstants m_LastConstants;
    bool m_HasLastConstants;

    // ModernBMD borrows one indexed UBO slot from the mixed legacy/Core frame.
    // Snapshot and restore the previous object instead of clearing the slot.
    mutable bool m_Bound;
    mutable unsigned int m_PreviousBaseBuffer;
};