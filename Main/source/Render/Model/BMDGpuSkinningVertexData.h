#pragma once

#include "BMDGpuSkinningVertexLayout.h"

namespace Render
{
    // Converts the current Season 5.2 BMD representation (one source bone
    // per vertex) into the four-influence GPU representation.
    inline BMDGpuSkinningVertexData BuildBMDGpuSkinningVertexData(int sourceBone)
    {
        BMDGpuSkinningVertexData result;

        if (sourceBone < 0)
            sourceBone = 0;
        if (sourceBone >= MaxGPUSkinBones)
            sourceBone = MaxGPUSkinBones - 1;

        result.boneIndices[0] = static_cast<unsigned int>(sourceBone);
        result.boneIndices[1] = 0u;
        result.boneIndices[2] = 0u;
        result.boneIndices[3] = 0u;

        result.boneWeights[0] = 1.0f;
        result.boneWeights[1] = 0.0f;
        result.boneWeights[2] = 0.0f;
        result.boneWeights[3] = 0.0f;

        return result;
    }

    inline bool ValidateBMDGpuSkinningVertexData(const BMDGpuSkinningVertexData& data)
    {
        float totalWeight = 0.0f;

        for (int i = 0; i < MaxGPUSkinInfluences; ++i)
        {
            if (data.boneIndices[i] >= static_cast<unsigned int>(MaxGPUSkinBones))
                return false;

            if (data.boneWeights[i] < 0.0f)
                return false;

            totalWeight += data.boneWeights[i];
        }

        return totalWeight <= 1.0001f && totalWeight >= 0.9999f;
    }
}
