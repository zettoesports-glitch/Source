#pragma once

#include <cstdint>

namespace Render
{
    // Transitional GPU vertex representation.
    // The current Season 5.2 BMD data contains one source bone per vertex.
    // We represent that faithfully as one active influence plus three neutral ones.
    struct BMDGpuSkinVertex
    {
        float position[3];
        float normal[3];
        float texcoord[2];
        uint32_t boneIndices[4];
        float boneWeights[4];

        BMDGpuSkinVertex()
        {
            for (int i = 0; i < 3; ++i)
            {
                position[i] = 0.0f;
                normal[i] = 0.0f;
            }

            texcoord[0] = 0.0f;
            texcoord[1] = 0.0f;

            boneIndices[0] = 0;
            boneIndices[1] = 0;
            boneIndices[2] = 0;
            boneIndices[3] = 0;

            boneWeights[0] = 1.0f;
            boneWeights[1] = 0.0f;
            boneWeights[2] = 0.0f;
            boneWeights[3] = 0.0f;
        }
    };

    inline void SetSingleBoneInfluence(BMDGpuSkinVertex& vertex, uint32_t sourceBone)
    {
        vertex.boneIndices[0] = sourceBone;
        vertex.boneIndices[1] = 0;
        vertex.boneIndices[2] = 0;
        vertex.boneIndices[3] = 0;

        vertex.boneWeights[0] = 1.0f;
        vertex.boneWeights[1] = 0.0f;
        vertex.boneWeights[2] = 0.0f;
        vertex.boneWeights[3] = 0.0f;
    }

    inline bool IsValidBoneIndex(uint32_t bone)
    {
        return bone < 200u;
    }

    inline void ClampBoneInfluence(BMDGpuSkinVertex& vertex)
    {
        if (!IsValidBoneIndex(vertex.boneIndices[0]))
            vertex.boneIndices[0] = 0;

        for (int i = 0; i < 4; ++i)
        {
            if (vertex.boneWeights[i] < 0.0f)
                vertex.boneWeights[i] = 0.0f;
        }

        float total = vertex.boneWeights[0] + vertex.boneWeights[1] +
                      vertex.boneWeights[2] + vertex.boneWeights[3];

        if (total <= 0.0f)
        {
            SetSingleBoneInfluence(vertex, vertex.boneIndices[0]);
            return;
        }

        const float inverseTotal = 1.0f / total;
        for (int i = 0; i < 4; ++i)
            vertex.boneWeights[i] *= inverseTotal;
    }
}
