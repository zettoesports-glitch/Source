#include "stdafx.h"
#include "BMDGpuSkinningBridge.h"
#include "../../ZzzBMD.h"

namespace Render
{
    bool BuildBMDGpuBonePalette(const BMD& model,
                                const float (*boneMatrix)[3][4],
                                BonePalette& outPalette)
    {
        ResetBonePalette(outPalette);

        if (boneMatrix == 0 || model.NumBones <= 0)
            return false;

        int count = static_cast<int>(model.NumBones);
        if (count > MaxGPUSkinBones)
            count = MaxGPUSkinBones;

        for (int i = 0; i < count; ++i)
        {
            if (!AppendBoneMatrix(outPalette, boneMatrix[i]))
                return false;
        }

        return outPalette.count > 0;
    }

    BMDVertexInfluence BuildBMDGpuInfluence(int sourceBone)
    {
        return BuildSingleBoneBMDInfluence(sourceBone);
    }
}
