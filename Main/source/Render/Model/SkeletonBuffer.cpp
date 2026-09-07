#include "stdafx.h"
#include "SkeletonBuffer.h"

#include <cstring>

SkeletonBuffer::SkeletonBuffer(StorageMode mode)
    : m_Mode(mode)
    , m_BoneCount(0)
    , m_Generation(1)
    , m_Dirty(false)
{
}

void SkeletonBuffer::BeginFrame()
{
    m_Texels.clear();
    m_BoneCount = 0;
    ++m_Generation;
    m_Dirty = true;
}

SkeletonBuffer::Allocation SkeletonBuffer::Allocate(std::uint32_t boneCount)
{
    if (boneCount == 0)
        return Allocation();

    Allocation allocation;
    allocation.BaseBone = m_BoneCount;
    allocation.BoneCount = boneCount;
    allocation.Generation = m_Generation;

    m_BoneCount += boneCount;
    const size_t floatCount = static_cast<size_t>(m_BoneCount)
        * GetTexelsPerBone() * 4u;
    m_Texels.resize(floatCount, 0.0f);
    m_Dirty = true;
    return allocation;
}

bool SkeletonBuffer::UploadAffine3x4(const Allocation& allocation,
                                     const float* matrices,
                                     std::uint32_t matrixCount)
{
    if (m_Mode != StorageMode::Matrix4x4 ||
        !allocation.IsValid() ||
        allocation.Generation != m_Generation ||
        matrices == nullptr ||
        matrixCount < allocation.BoneCount)
        return false;

    for (std::uint32_t bone = 0; bone < allocation.BoneCount; ++bone)
    {
        const float* source = matrices + static_cast<size_t>(bone) * 12u;
        float* destination = m_Texels.data()
            + static_cast<size_t>(allocation.BaseBone + bone) * MatrixFloatsPerBone;

        std::memcpy(destination, source, sizeof(float) * 12u);
        destination[12] = 0.0f;
        destination[13] = 0.0f;
        destination[14] = 0.0f;
        destination[15] = 1.0f;
    }

    m_Dirty = true;
    return true;
}