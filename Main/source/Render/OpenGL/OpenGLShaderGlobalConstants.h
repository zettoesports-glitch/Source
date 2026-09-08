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
};
