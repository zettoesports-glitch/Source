#include "stdafx.h"
#include "BMDModernInstanceBuilder.h"

#include <cstring>

BMDModernInstanceParams::BMDModernInstanceParams()
    : Translate(false)
    , BodyScale(1.0f)
    , NormalOffset(0.0f)
    , EnableLight(false)
    , MinAlpha(0.0f)
    , EnableShadow(true)
    , ShadowHeight(0.0f)
    , BoneIndex(0)
{
    BodyOrigin[0] = BodyOrigin[1] = BodyOrigin[2] = 0.0f;
    BlendUV[0] = BlendUV[1] = 0.0f;

    BodyLight[0] = BodyLight[1] = BodyLight[2] = BodyLight[3] = 1.0f;
    ShadowLight[0] = ShadowLight[1] = ShadowLight[2] = ShadowLight[3] = 0.0f;
}

BMDModernInstance BMDModernInstanceBuilder::Build(
    const BMDModernInstanceParams& params)
{
    BMDModernInstance out;
    std::memset(&out, 0, sizeof(out));

    if (params.Translate)
    {
        out.BodyOrigin[0] = params.BodyOrigin[0];
        out.BodyOrigin[1] = params.BodyOrigin[1];
        out.BodyOrigin[2] = params.BodyOrigin[2];
        out.BodyScale[0] = params.BodyScale;
    }
    else
    {
        out.BodyScale[0] = 1.0f;
    }

    // The reference shader's second BodyScale component is a normal-direction
    // offset. Do not feed the legacy multiplicative global BoneScale here.
    out.BodyScale[1] = params.NormalOffset;

    out.Data[0] = params.EnableLight ? 1.0f : 0.0f;
    out.Data[1] = params.MinAlpha;

    out.Data2[0] = params.BlendUV[0];
    out.Data2[1] = params.BlendUV[1];
    out.Data2[2] = params.EnableShadow ? 1.0f : 0.0f;
    out.Data2[3] = params.ShadowHeight;

    std::memcpy(out.BodyLight, params.BodyLight, sizeof(out.BodyLight));
    std::memcpy(out.ShadowLight, params.ShadowLight, sizeof(out.ShadowLight));

    // BoneIndex is stored in BONE UNITS. RequestBone() performs + BoneId and
    // then converts to the physical texel offset according to shader mode.
    out.BoneIndex = params.BoneIndex;
    return out;
}
