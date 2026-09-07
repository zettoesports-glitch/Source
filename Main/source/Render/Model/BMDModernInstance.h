#pragma once

#include <cstdint>

// Backend-neutral per-instance contract from VertexDefinitions.inc.
// Locations are configured by the backend; fields stay in shader order.
struct BMDModernInstance
{
    float BodyOrigin[3];   // location 6
    float BodyScale[2];    // location 7: x = BodyScale, y = BoneScale/normal offset
    float Data[2];         // location 8: x = EnableLight, y = MinAlpha
    float Data2[4];        // location 9: xy = UV blend, z = shadow enable, w = shadow height
    float BodyLight[4];    // location 10: xyz = light, w = alpha
    float ShadowLight[4];  // location 11
    std::uint32_t BoneIndex; // location 12, expressed in BONE UNITS
};
