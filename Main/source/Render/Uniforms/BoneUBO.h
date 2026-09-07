#pragma once

#include <cstdint>

namespace Render
{
    static const uint32_t MaxBones = 200;
    static const uint32_t BoneUBOSlot = 2;

    struct alignas(16) BoneUBO
    {
        float matrices[MaxBones][4][4];
    };
}
