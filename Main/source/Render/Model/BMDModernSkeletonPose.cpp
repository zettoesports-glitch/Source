#include "stdafx.h"
#include "BMDModernSkeletonPose.h"
#include "SkeletonBuffer.h"

#include <cmath>
#include <cstring>

namespace
{
    // Validation-phase safety guard. The current capture path feeds the modern
    // atlas from the legacy GPU palette, where object BodyScale/requestScale can
    // already be baked into the 3x3 basis. The generated shaders expect model
    // scale to be handled separately, so accepting a strongly non-unit basis can
    // deform geometry. Until the transform split has CPU/GPU parity tests, keep
    // those poses on the legacy renderer instead of drawing corrupted meshes.
    static bool HasNearUnitRootBasis(const float* matrices)
    {
        if (matrices == NULL)
            return false;

        const float sx = std::sqrt(matrices[0] * matrices[0] +
                                   matrices[1] * matrices[1] +
                                   matrices[2] * matrices[2]);
        const float sy = std::sqrt(matrices[4] * matrices[4] +
                                   matrices[5] * matrices[5] +
                                   matrices[6] * matrices[6]);
        const float sz = std::sqrt(matrices[8] * matrices[8] +
                                   matrices[9] * matrices[9] +
                                   matrices[10] * matrices[10]);

        if (!std::isfinite(sx) || !std::isfinite(sy) || !std::isfinite(sz))
            return false;

        const float tolerance = 0.02f;
        return std::fabs(sx - 1.0f) <= tolerance &&
               std::fabs(sy - 1.0f) <= tolerance &&
               std::fabs(sz - 1.0f) <= tolerance;
    }
}

bool BMDModernSkeletonPose::BuildAdjustedAffine3x4(
    const float* boneMatrices,
    std::uint32_t boneCount,
    float requestScale,
    std::vector<float>& outMatrices)
{
    outMatrices.clear();
    if (boneMatrices == NULL || boneCount == 0 || !std::isfinite(requestScale))
        return false;

    if (!HasNearUnitRootBasis(boneMatrices))
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
