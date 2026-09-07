#pragma once

#if jdk_shader_local330

#include "New_ModelBMD.h"

class BMDModernRuntime
{
public:
    static BMDModernRuntime& Instance();

    bool IsEnabled() const;

    // Deliberately narrow first-light path. It stages the current BMD's final
    // BoneTransform into BonesTexture and draws only the first eligible base
    // textured BMD. False always means: execute the legacy renderer unchanged.
    bool TryRender(const OGL330MODEL::RenderMeshVAO& command);

private:
    BMDModernRuntime();
    ~BMDModernRuntime();
    BMDModernRuntime(const BMDModernRuntime&);
    BMDModernRuntime& operator=(const BMDModernRuntime&);

    struct Impl;
    Impl* m_Impl;
};

#define gBMDModernRuntime (BMDModernRuntime::Instance())

#endif // jdk_shader_local330
