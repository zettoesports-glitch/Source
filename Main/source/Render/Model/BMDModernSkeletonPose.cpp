#include "stdafx.h"
#include "BMDModernSkeletonPose.h"
#include "SkeletonBuffer.h"

#include <cmath>
#include <cstring>

bool BMDModernSkeletonPose::BuildAdjustedAffine3x4(
    const float* boneMatrices,
    std::uint32_t boneCount,
    float requestScale,
    std::vector<float>& outMatrices)
{
    outMatrices.clear();
    if (boneMatrices == NULL || boneCount == 0 || !std::isfinite(requestScale))
        return false;

    const float localScale =
        (requestScale == 0.0f || requestScale == 1.0f) ? 1.0f : requestScale;

    outMatrices.resize(static_cast<size_t>(boneCount) * 12u);
    std::memcpy(outMatrices.data(),
                boneMatrices,
                outMatrices.size() * sizeof(float));

    if (localScale == 1.0f)
        return true;

    for (std::uint32_t bone = 0; bone < boneCount; ++bone)
    {
        float* matrix = outMatrices.data() + static_cast<size_t>(bone) * 12u;

        // Match the legacy GPU palette path: scale the 3x3 basis/local vertex
        // contribution, but never the bone translation in [3], [7], [11].
        matrix[0] *= localScale;
        matrix[1] *= localScale;
        matrix[2] *= localScale;
        matrix[4] *= localScale;
        matrix[5] *= localScale;
        matrix[6] *= localScale;
        matrix[8] *= localScale;
        matrix[9] *= localScale;
        matrix[10] *= localScale;
    }

    return true;
}

BMDModernSkeletonSubmission BMDModernSkeletonPose::Stage(
    SkeletonBuffer& skeletonBuffer,
    const float* boneMatrices,
    std::uint32_t boneCount,
    float requestScale)
{
    BMDModernSkeletonSubmission result;

    std::vector<float> adjustedMatrices;
    if (!BuildAdjustedAffine3x4(boneMatrices,
                                boneCount,
                                requestScale,
                                adjustedMatrices))
        return result;

    const SkeletonBuffer::Allocation allocation = skeletonBuffer.Allocate(boneCount);
    if (!allocation.IsValid())
        return result;

    if (!skeletonBuffer.UploadAffine3x4(allocation,
                                       adjustedMatrices.data(),
                                       boneCount))
        return result;

    result.BoneIndex = allocation.BaseBone;
    result.BoneCount = allocation.BoneCount;
    result.Generation = allocation.Generation;
    result.Success = true;
    return result;
}
