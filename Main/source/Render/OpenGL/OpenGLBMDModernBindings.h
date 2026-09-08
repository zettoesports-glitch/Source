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

    // ModernBMD runs inside a renderer that still mixes legacy OpenGL state
    // with Core-style explicit bindings. Keep the exact texture state that was
    // active before a modern draw and restore it afterwards instead of merely
    // binding texture 0. This mirrors Sven's BindState ownership principle
    // while retaining our strict modern/legacy isolation boundary.
    mutable bool m_Bound;
    mutable int m_PreviousActiveTexture;
    mutable unsigned int m_PreviousMaterialTexture;
    mutable unsigned int m_PreviousSkeletonTexture;
};
