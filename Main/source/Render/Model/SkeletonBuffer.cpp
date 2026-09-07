#include "stdafx.h"
#include "SkeletonBuffer.h"

#include <cstring>

SkeletonBuffer::SkeletonBuffer()
    : m_BoneCount(0)
    , m_Generation(1)
    , m_Dirty(false)
{
}

void SkeletonBuffer::BeginFrame()
{
    // Allocations are frame-local so BoneIndex values remain compact and the
    // backend can upload one contiguous skeleton resource.
    m_Texels.clear();
    m_BoneCount = 0;
    ++m_Generation;
    m_Dirty = true;
}

SkeletonBuffer::Allocation SkeletonBuffer::Allocate(std::uint32_t boneCount)
{
    if (boneCount == 0)
    {
        return Allocation();
    }

    Allocation allocation;
    allocation.BaseBone = m_BoneCount;
    allocation.BoneCount = boneCount;
    allocation.Generation = m_Generation;

    m_BoneCount += boneCount;
    m_Texels.resize(static_cast<size_t>(m_BoneCount) * MatrixFloatsPerBone, 0.0f);
    m_Dirty = true;

    return allocation;
}

bool SkeletonBuffer::UploadAffine3x4(const Allocation& allocation,
                                     const float* matrices,
                                     std::uint32_t matrixCount)
{
    if (!allocation.IsValid() ||
        allocation.Generation != m_Generation ||
        matrices == nullptr ||
        matrixCount < allocation.BoneCount)
    {
        return false;
    }

    for (std::uint32_t bone = 0; bone < allocation.BoneCount; ++bone)
    {
        const float* source = matrices + static_cast<size_t>(bone) * 12;
        float* destination = m_Texels.data()
            + static_cast<size_t>(allocation.BaseBone + bone) * MatrixFloatsPerBone;

        // Matrix rows expected by RequestBone().
        std::memcpy(destination, source, sizeof(float) * 12);

        // Affine homogeneous row.
        destination[12] = 0.0f;
        destination[13] = 0.0f;
        destination[14] = 0.0f;
        destination[15] = 1.0f;
    }

    m_Dirty = true;
    return true;
}
