#include "stdafx.h"
#include "BMDModernSkeletonAtlas.h"

BMDModernSkeletonAtlas::BMDModernSkeletonAtlas(SkeletonBuffer::StorageMode mode)
    : m_Buffer(mode)
{
}

void BMDModernSkeletonAtlas::BeginFrame()
{
    m_Buffer.BeginFrame();
    m_Entries.clear();
    m_Stats = Stats();
}

BMDModernSkeletonSubmission BMDModernSkeletonAtlas::Stage(
    const void* poseKey,
    const float* boneMatrices,
    std::uint32_t boneCount,
    float requestScale)
{
    BMDModernSkeletonSubmission failure;
    if (poseKey == NULL || boneMatrices == NULL || boneCount == 0)
    {
        ++m_Stats.FailedPoseCount;
        return failure;
    }

    const std::unordered_map<const void*, BMDModernSkeletonSubmission>::const_iterator found =
        m_Entries.find(poseKey);
    if (found != m_Entries.end())
    {
        ++m_Stats.ReusedPoseCount;
        return found->second;
    }

    const BMDModernSkeletonSubmission submission = BMDModernSkeletonPose::Stage(
        m_Buffer,
        boneMatrices,
        boneCount,
        requestScale);
    if (!submission.Success)
    {
        ++m_Stats.FailedPoseCount;
        return failure;
    }

    m_Entries.insert(std::make_pair(poseKey, submission));
    ++m_Stats.PoseCount;
    m_Stats.BoneCount += submission.BoneCount;
    return submission;
}

const BMDModernSkeletonSubmission* BMDModernSkeletonAtlas::Find(const void* poseKey) const
{
    if (poseKey == NULL)
        return NULL;

    const std::unordered_map<const void*, BMDModernSkeletonSubmission>::const_iterator found =
        m_Entries.find(poseKey);
    if (found == m_Entries.end())
        return NULL;

    return &found->second;
}
