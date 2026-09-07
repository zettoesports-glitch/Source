#pragma once

#include "BMDModernInstance.h"

struct BMDModernInstanceParams
{
    bool Translate;
    float BodyScale;
    float BodyOrigin[3];
    float NormalOffset;

    bool EnableLight;
    float MinAlpha;
    float BlendUV[2];

    bool EnableShadow;
    float ShadowHeight;

    float BodyLight[4];
    float ShadowLight[4];

    std::uint32_t BoneIndex;

    BMDModernInstanceParams();
};

class BMDModernInstanceBuilder
{
public:
    static BMDModernInstance Build(const BMDModernInstanceParams& params);
};
