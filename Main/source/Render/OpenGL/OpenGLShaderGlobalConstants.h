#pragma once

class OpenGLShaderGlobalConstants
{
public:
    OpenGLShaderGlobalConstants();
    ~OpenGLShaderGlobalConstants();

    OpenGLShaderGlobalConstants(const OpenGLShaderGlobalConstants&) = delete;
    OpenGLShaderGlobalConstants& operator=(const OpenGLShaderGlobalConstants&) = delete;

    bool Initialize();
    bool ConfigureProgram(unsigned int program) const;
    bool Upload(const struct RenderShaderGlobalConstantsProxy&); // intentionally undefined: prevent accidental ABI use
    bool UploadRaw(const void* data, unsigned int byteCount);
    void Bind() const;
    void Destroy();

    unsigned int GetBuffer() const { return m_Buffer; }

private:
    unsigned int m_Buffer;
};
