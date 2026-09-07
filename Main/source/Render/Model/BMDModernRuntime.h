#pragma once

#if jdk_shader_local330

#include "New_ModelBMD.h"

class BMDModernRuntime
{
public:
    static BMDModernRuntime& Instance();

    bool IsEnabled() const;

    // Builds one frame-local skeleton atlas from the raw final/global MU
    // BoneTransform matrices captured by the legacy command frontend.
    void PrepareFrame(const OGL330MODEL::MeshVAO& commands);

    // Attempts the deliberately narrow first modern BMD draw. Returning false
    // means the caller must execute the legacy renderer unchanged.
    bool TryRender(const OGL330MODEL::RenderMeshVAO& command);

    void EndFrame();

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
