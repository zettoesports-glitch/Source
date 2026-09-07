#pragma once

#include "BMDModernSkeletonPose.h"
#include "SkeletonBuffer.h"

#include <cstdint>
#include <unordered_map>

// Frame-local atlas for modern BMD skeletons. The atlas owns one shared
// SkeletonBuffer and assigns each unique pose a BaseBone in BONE UNITS.
// OpenGL and Vulkan backends can upload the resulting buffer once per batch.
class BMDModernSkeletonAtlas
{
public:
    struct Stats
    {
        std::uint32_t PoseCount = 0;
        std::uint32_t BoneCount = 0;
        std::uint32_t ReusedPoseCount = 0;
        std::uint32_t FailedPoseCount = 0;
    };

    explicit BMDModernSkeletonAtlas(
        SkeletonBuffer::StorageMode mode = SkeletonBuffer::StorageMode::QuaternionPositionScale);

    // Invalidates all frame-local handles and starts one contiguous skeleton
    // allocation space. Call exactly once before staging the batch.
    void BeginFrame();

    // poseKey identifies an immutable pose snapshot for this batch. Reusing the
    // same key returns the same BaseBone without encoding or allocating again.
    BMDModernSkeletonSubmission Stage(
        const void* poseKey,
        const float* boneMatrices,
        std::uint32_t boneCount,
        float requestScale);

    const BMDModernSkeletonSubmission* Find(const void* poseKey) const;

    SkeletonBuffer& GetBuffer() { return m_Buffer; }
    const SkeletonBuffer& GetBuffer() const { return m_Buffer; }
    const Stats& GetStats() const { return m_Stats; }

private:
    SkeletonBuffer m_Buffer;
    std::unordered_map<const void*, BMDModernSkeletonSubmission> m_Entries;
    Stats m_Stats;
};
