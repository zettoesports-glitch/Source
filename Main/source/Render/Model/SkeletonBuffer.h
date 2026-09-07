#pragma once

#include <cstdint>
#include <vector>

// Backend-neutral skeleton storage.
//
// The shader contract is:
//   RequestBone(BoneIndex, BoneId)
// where BoneIndex is the base bone of an allocation and each bone occupies
// four float4 texels (or two float4 texels when dual quaternions are enabled).
//
// This frontend intentionally owns no OpenGL or Vulkan objects. Backends upload
// GetTexels() to their native BonesTexture resource.
class SkeletonBuffer
{
public:
    static constexpr std::uint32_t MatrixTexelsPerBone = 4;
    static constexpr std::uint32_t MatrixFloatsPerBone = 16;

    struct Allocation
    {
        std::uint32_t BaseBone = 0;
        std::uint32_t BoneCount = 0;
        std::uint64_t Generation = 0;

        bool IsValid() const { return BoneCount != 0; }
    };

    SkeletonBuffer();

    void BeginFrame();

    // Allocate a contiguous bone range. The returned BaseBone is passed to the
    // instance as BoneIndex and is added to the mesh-local BoneId in the shader.
    Allocation Allocate(std::uint32_t boneCount);

    // Upload affine 3x4 matrices. Each source bone contains 12 floats:
    // [row0.xyzw][row1.xyzw][row2.xyzw].
    // Storage is expanded to four float4 texels with the final affine row.
    bool UploadAffine3x4(const Allocation& allocation,
                         const float* matrices,
                         std::uint32_t matrixCount);

    const std::vector<float>& GetTexels() const { return m_Texels; }
    std::uint64_t GetGeneration() const { return m_Generation; }
    bool IsDirty() const { return m_Dirty; }
    void ClearDirty() { m_Dirty = false; }

    std::uint32_t GetBoneCount() const { return m_BoneCount; }

private:
    std::vector<float> m_Texels;
    std::uint32_t m_BoneCount;
    std::uint64_t m_Generation;
    bool m_Dirty;
};
