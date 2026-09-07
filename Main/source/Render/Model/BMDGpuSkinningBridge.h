#pragma once

#include "GPUSkinning.h"
#include "BMDSkinning.h"

class BMD;

namespace Render
{
    // Adapter between the existing MU animation representation and the
    // renderer-neutral GPU palette. It does not change legacy animation.
    bool BuildBMDGpuBonePalette(const BMD& model,
                                const float (*boneMatrix)[3][4],
                                BonePalette& outPalette);

    // Current BMD files expose one source bone per vertex (Vertex_t::Node).
    // Keep that representation lossless while producing the four-influence
    // GPU layout; additional influences can be populated later when the
    // asset format supplies explicit weights.
    BMDVertexInfluence BuildBMDGpuInfluence(int sourceBone);
}
