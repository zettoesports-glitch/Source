#pragma once

#include <cstdint>
#include <vector>

// Backend-neutral skeleton storage. The physical layout must match the shader
// variant selected by the renderer.
class SkeletonBuffer
{
public:
    enum class StorageMode : std::uint32_t
    {
        Matrix4x4 = 4,       // four float4 texels per bone
        DualQuaternion = 2  // rotation quaternion + position/scale
    };

    static constexpr std::uint32_t MatrixTexelsPerBone = 4;
    static constexpr std::uint32_t DualQuatTexelsPerBone = 2;
    static constexpr std::uint32_t MatrixFloatsPerBone = 16;

    struct Allocation
    {
        std::uint32_t BaseBone = 0;
        std::uint32_t BoneCount = 0;
        std::uint64_t Generation = 0;
        bool IsValid() const { return BoneCount != 0; }
    };

    explicit SkeletonBuffer(StorageMode mode = StorageMode::Matrix4x4);

    void BeginFrame();
    Allocation Allocate(std::uint32_t boneCount);

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