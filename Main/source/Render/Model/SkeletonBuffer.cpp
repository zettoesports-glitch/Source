#include "stdafx.h"
#include "SkeletonBuffer.h"
#include "SkeletonEncoder.h"

#include <algorithm>

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
    if (!allocation.IsValid() ||
        allocation.Generation != m_Generation ||
        matrices == nullptr ||
        matrixCount < allocation.BoneCount ||
        allocation.BaseBone + allocation.BoneCount > m_BoneCount)
    {
        return false;
    }

    // SkeletonEncoder is the single source of truth for CPU BMD affine memory
    // -> shader texel layout. Do not memcpy the 3x4 matrix directly here:
    // Matrix4x4 requires an explicit transpose and QPS requires decomposition.
    std::vector<float> encoded;
    switch (m_Mode)
    {
    case StorageMode::Matrix4x4:
        SkeletonEncoder::EncodeMatrix4x4(matrices, allocation.BoneCount, encoded);
        break;

    case StorageMode::QuaternionPositionScale:
        if (!SkeletonEncoder::EncodeQuaternionPositionScale(
                matrices, allocation.BoneCount, encoded))
        {
            return false;
        }
        break;

    default:
        return false;
    }

    const size_t floatsPerBone = static_cast<size_t>(GetTexelsPerBone()) * 4u;
    const size_t expectedFloats = static_cast<size_t>(allocation.BoneCount) * floatsPerBone;
    if (encoded.size() != expectedFloats)
        return false;

    const size_t destinationOffset = static_cast<size_t>(allocation.BaseBone) * floatsPerBone;
    std::copy(encoded.begin(), encoded.end(), m_Texels.begin() + destinationOffset);

    m_Dirty = true;
    return true;
}
