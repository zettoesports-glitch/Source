#pragma once

#include "GPUSkinning.h"

namespace Render
{
    // CPU-side representation of the std140 BoneBlock used by the GPU path.
    // A mat4 is 16 floats in std140, so BMD's native 3x4 matrices must be
    // expanded before being uploaded to the real GL uniform buffer.
    struct BMDGpuBoneBlock
    {
        float matrices[MaxGPUSkinBones][16];

        BMDGpuBoneBlock()
        {
            for (int bone = 0; bone < MaxGPUSkinBones; ++bone)
            {
                for (int value = 0; value < 16; ++value)
                    matrices[bone][value] = 0.0f;
                matrices[bone][15] = 1.0f;
            }
        }
    };

    inline int BuildBMDGpuBoneBlock(const BonePalette& palette,
                                    BMDGpuBoneBlock& outBlock)
    {
        int count = palette.count;
        if (count < 0)
            count = 0;
        if (count > MaxGPUSkinBones)
            count = MaxGPUSkinBones;

        for (int bone = 0; bone < count; ++bone)
        {
            BoneMatrix4x4 matrix;
            ConvertBoneMatrixTo4x4(palette.bones[bone], matrix);

            for (int value = 0; value < 16; ++value)
                outBlock.matrices[bone][value] = matrix.m[value];
        }

        for (int bone = count; bone < MaxGPUSkinBones; ++bone)
        {
            for (int value = 0; value < 16; ++value)
                outBlock.matrices[bone][value] = 0.0f;
            outBlock.matrices[bone][15] = 1.0f;
        }

        return count;
    }
}
