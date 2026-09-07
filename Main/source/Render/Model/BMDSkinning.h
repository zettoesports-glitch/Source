#pragma once

#include "GPUSkinning.h"

namespace Render
{
    struct BMDVertexInfluence
    {
        unsigned char bone[MaxGPUSkinInfluences];
        float weight[MaxGPUSkinInfluences];
    };

    inline void BuildBMDVertexInfluence(BMDVertexInfluence& influence,
                                        int bone0, float weight0,
                                        int bone1 = 0, float weight1 = 0.0f,
                                        int bone2 = 0, float weight2 = 0.0f,
                                        int bone3 = 0, float weight3 = 0.0f)
    {
        const int bones[MaxGPUSkinInfluences] = { bone0, bone1, bone2, bone3 };
        const float weights[MaxGPUSkinInfluences] = { weight0, weight1, weight2, weight3 };

        for (int i = 0; i < MaxGPUSkinInfluences; ++i)
        {
            int index = bones[i];
            if (index < 0) index = 0;
            if (index >= MaxGPUSkinBones) index = MaxGPUSkinBones - 1;
            influence.bone[i] = static_cast<unsigned char>(index);
            influence.weight[i] = weights[i];
        }
    }

    inline void NormalizeBMDInfluence(BMDVertexInfluence& influence)
    {
        float sum = 0.0f;
        for (int i = 0; i < MaxGPUSkinInfluences; ++i)
        {
            if (influence.weight[i] < 0.0f)
                influence.weight[i] = 0.0f;
            sum += influence.weight[i];
        }

        if (sum <= 0.0f)
        {
            influence.weight[0] = 1.0f;
            influence.weight[1] = 0.0f;
            influence.weight[2] = 0.0f;
            influence.weight[3] = 0.0f;
            return;
        }

        const float inv = 1.0f / sum;
        for (int i = 0; i < MaxGPUSkinInfluences; ++i)
            influence.weight[i] *= inv;
    }

    inline BMDVertexInfluence BuildSingleBoneBMDInfluence(int bone)
    {
        BMDVertexInfluence influence;
        BuildBMDVertexInfluence(influence, bone, 1.0f);
        NormalizeBMDInfluence(influence);
        return influence;
    }
}
