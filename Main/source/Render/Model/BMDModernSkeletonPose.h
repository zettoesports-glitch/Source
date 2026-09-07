#pragma once

#include <cstdint>
#include <vector>

class SkeletonBuffer;

struct BMDModernSkeletonSubmission
{
    std::uint32_t BoneIndex = 0;
    std::uint32_t BoneCount = 0;
    std::uint64_t Generation = 0;
    bool Success = false;
};

// Prepares the final/global MU BoneTransform matrices for the shared skeleton
// buffer without baking BodyScale or BodyOrigin into the skeleton.
class BMDModernSkeletonPose
{
public:
    // requestScale matches the legacy GPU path's m_fRequestScale behavior:
    // 0 or 1 means no extra local scale. Any other value multiplies only the
    // 3x3 basis; bone translation remains unchanged.
    static bool BuildAdjustedAffine3x4(const float* boneMatrices,
                                       std::uint32_t boneCount,
                                       float requestScale,
                                       std::vector<float>& outMatrices);

    static BMDModernSkeletonSubmission Stage(SkeletonBuffer& skeletonBuffer,
                                             const float* boneMatrices,
                                             std::uint32_t boneCount,
                                             float requestScale);
};
