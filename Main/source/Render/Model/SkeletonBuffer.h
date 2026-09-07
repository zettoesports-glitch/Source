#pragma once

#include <cstdint>
#include <vector>

// Backend-neutral skeleton staging storage. The physical texel layout must
// match the shader variant selected by the renderer.
class SkeletonBuffer
{
public:
    enum class StorageMode : std::uint32_t
    {
        Matrix4x4 = 4,                // four float4 texels per bone
        QuaternionPositionScale = 2, // quaternion + translation/uniform-scale
        DualQuaternion = QuaternionPositionScale // legacy naming compatibility
    };

    static constexpr std::uint32_t MatrixTexelsPerBone = 4;
    static constexpr std::uint32_t QuaternionPositionScaleTexelsPerBone = 2;
    static constexpr std::uint32_t DualQuatTexelsPerBone = QuaternionPositionScaleTexelsPerBone;
    static constexpr std::uint32_t MatrixFloatsPerBone = 16;

    struct Allocation
    {
        std::uint32_t BaseBone = 0;
        std::uint32_t BoneCount = 0;
        std::uint64_t Generation = 0;
        bool IsValid() const { return BoneCount != 0; }
    };

    explicit SkeletonBuffer(StorageMode mode = StorageMode::Matrix4x4);

    // Frame-local allocator. BaseBone is expressed in BONE UNITS, exactly as
    // expected by shader input BoneIndex; the shader converts it to texels.
    void BeginFrame();
    Allocation Allocate(std::uint32_t boneCount);

    // Encodes final/global MU affine 3x4 bone matrices into the selected shader
    // storage format and writes them into the allocation. QPS mode rejects
    // degenerate, non-uniform-scale, or sheared matrices.
    bool UploadAffine3x4(const Allocation& allocation,
                         const float* matrices,
                         std::uint32_t matrixCount);

    const std::vector<float>& GetTexels() const { return m_Texels; }
    StorageMode GetStorageMode() const { return m_Mode; }
    std::uint32_t GetTexelsPerBone() const { return static_cast<std::uint32_t>(m_Mode); }
    std::uint64_t GetGeneration() const { return m_Generation; }
    bool IsDirty() const { return m_Dirty; }
    void ClearDirty() { m_Dirty = false; }
    std::uint32_t GetBoneCount() const { return m_BoneCount; }

private:
    std::vector<float> m_Texels;
    StorageMode m_Mode;
    std::uint32_t m_BoneCount;
    std::uint64_t m_Generation;
    bool m_Dirty;
};