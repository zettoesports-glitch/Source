#pragma once

#if jdk_shader_local330

#include "New_ModelBMD.h"

class BMDModernRuntime
{
public:
    static BMDModernRuntime& Instance();

    bool IsEnabled() const;
    bool IsBatchPrepared() const;

    // Material-only compatibility check used by the full-object coherence guard.
    // Keep renderer/material knowledge in one place so adding a new modern pass
    // cannot accidentally leave FlushAllMesh and TryRender with different lists.
    bool IsMaterialCompatible(const OGL330MODEL::RenderMeshVAO& command) const;

    // Pre-stages every eligible immutable command palette into one contiguous
    // skeleton atlas and uploads BonesTexture once for the whole FlushAllMesh().
    // False keeps the entire batch on the unchanged legacy renderer.
    bool PrepareBatch(const OGL330MODEL::MeshVAO& commands);
    void FinishBatch();

    // Draws one command using the BaseBone prepared by PrepareBatch(). Any
    // unsupported material/resource returns false and immediately falls back to
    // the existing u_Bones renderer for that command.
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
