#pragma once

class OpenGLSkeletonTexture;

class OpenGLBMDModernBindings
{
public:
    static const unsigned int MaterialTextureUnit = 0;
    static const unsigned int SkeletonTextureUnit = 1;

    OpenGLBMDModernBindings();

    bool ConfigureProgram(unsigned int program);
    bool Bind(unsigned int materialTexture,
              const OpenGLSkeletonTexture& skeletonTexture) const;
    void Unbind() const;

    unsigned int GetProgram() const { return m_Program; }
    bool IsConfigured() const { return m_Program != 0; }

private:
    unsigned int m_Program;
    int m_MaterialSamplerLocation;
    int m_SkeletonSamplerLocation;
};
