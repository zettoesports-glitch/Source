#include "stdafx.h"
#include "BMDModernSkeletonAtlas.h"

namespace BMDModernSkeletonAtlasTests
{
    bool Run()
    {
        BMDModernSkeletonAtlas atlas(
            SkeletonBuffer::StorageMode::QuaternionPositionScale);
        atlas.BeginFrame();

        const float poseA[] =
        {
            1,0,0, 1,
            0,1,0, 2,
            0,0,1, 3,

            1,0,0, 4,
            0,1,0, 5,
            0,0,1, 6
        };

        const float poseB[] =
        {
            0,-1,0, 7,
            1, 0,0, 8,
            0, 0,1, 9
        };

        int keyA = 1;
        int keyB = 2;

        const BMDModernSkeletonSubmission a = atlas.Stage(&keyA, poseA, 2, 1.0f);
        if (!a.Success || a.BoneIndex != 0 || a.BoneCount != 2)
            return false;

        const BMDModernSkeletonSubmission b = atlas.Stage(&keyB, poseB, 1, 1.0f);
        if (!b.Success || b.BoneIndex != 2 || b.BoneCount != 1)
            return false;

        const BMDModernSkeletonSubmission aAgain = atlas.Stage(&keyA, poseA, 2, 1.0f);
        if (!aAgain.Success || aAgain.BoneIndex != a.BoneIndex ||
            aAgain.Generation != a.Generation)
        {
            return false;
        }

        const BMDModernSkeletonAtlas::Stats& stats = atlas.GetStats();
        if (stats.PoseCount != 2 || stats.BoneCount != 3 ||
            stats.ReusedPoseCount != 1 || stats.FailedPoseCount != 0)
        {
            return false;
        }

        if (atlas.GetBuffer().GetBoneCount() != 3)
            return false;

        const size_t expectedFloats =
            3u * SkeletonBuffer::QuaternionPositionScaleTexelsPerBone * 4u;
        if (atlas.GetBuffer().GetTexels().size() != expectedFloats)
            return false;

        const std::uint64_t generation = a.Generation;
        atlas.BeginFrame();
        if (atlas.Find(&keyA) != NULL || atlas.GetBuffer().GetBoneCount() != 0)
            return false;

        const BMDModernSkeletonSubmission next = atlas.Stage(&keyA, poseA, 2, 1.0f);
        if (!next.Success || next.BoneIndex != 0 || next.Generation == generation)
            return false;

        return true;
    }
}
