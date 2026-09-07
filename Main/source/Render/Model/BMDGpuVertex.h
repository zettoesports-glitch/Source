#pragma once

#include <stddef.h>

namespace Render
{
    // GPU vertex contract for the BMD migration.
    // The current Season 5.2 asset format supplies one bone per vertex;
    // influence 0 therefore carries the full weight and the remaining
    // influences are reserved for future weighted assets.
    struct BMDGpuVertex
    {
        float position[3];
        float normal[3];
        float texCoord[2];
        unsigned int boneIndex[4];
        float boneWeight[4];

        BMDGpuVertex()
        {
            position[0] = position[1] = position[2] = 0.0f;
            normal[0] = normal[1] = normal[2] = 0.0f;
            texCoord[0] = texCoord[1] = 0.0f;
            boneIndex[0] = boneIndex[1] = boneIndex[2] = boneIndex[3] = 0;
            boneWeight[0] = 1.0f;
            boneWeight[1] = boneWeight[2] = boneWeight[3] = 0.0f;
        }
    };

    inline void BuildSingleBoneGpuVertex(BMDGpuVertex& vertex, unsigned int bone)
    {
        vertex.boneIndex[0] = bone;
        vertex.boneIndex[1] = 0;
        vertex.boneIndex[2] = 0;
        vertex.boneIndex[3] = 0;
        vertex.boneWeight[0] = 1.0f;
        vertex.boneWeight[1] = 0.0f;
        vertex.boneWeight[2] = 0.0f;
        vertex.boneWeight[3] = 0.0f;
    }

    // Keep the GPU layout explicit so a VAO migration cannot silently drift.
    inline bool IsBMDGpuVertexLayoutValid()
    {
        return offsetof(BMDGpuVertex, position) == 0 &&
               offsetof(BMDGpuVertex, normal) == sizeof(float) * 3 &&
               offsetof(BMDGpuVertex, texCoord) == sizeof(float) * 6 &&
               offsetof(BMDGpuVertex, boneIndex) == sizeof(float) * 8 &&
               offsetof(BMDGpuVertex, boneWeight) == sizeof(float) * 12;
    }
}
