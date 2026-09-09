#pragma once
#include "RendererCommon.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>
namespace reverse_gl46 {

struct ModelMaterialState {
    bool textureEnabled{true};
    bool alphaCutoffEnabled{};
    float alphaReference{};
    int chromeMode{};
    float wave1{};
    float wave2{};
    Vec3 chromeL{};
    Vec2 blendMeshTexCoord{};
    bool fogEnabled{};
    Vec3 fogColor{};
    float fogStart{};
    float fogEnd{};
    float fogIntensity{1.0f};
};

struct ModelPose {
    // EXTRACTED: uniform mat4 u_Bones[200]
    std::array<Mat4, 200> bones{};
    std::uint32_t activeBones{};
};

struct ModelInstanceState {
    Vec4 bodyLight{1,1,1,1};
    Vec4 lightPosition{0,0,1,0};
    bool enableLight{true};
    Vec3 bodyOrigin{};
    Vec3 sunOffset{2000.0f, 0.0f, 4000.0f};
};

struct ModelDrawCommand {
    std::uint32_t program{};
    std::uint32_t vertexArray{};
    std::uint32_t indexBuffer{};
    std::uint32_t texture{};
    std::uint32_t firstIndex{};
    std::uint32_t indexCount{};
    bool transparent{};
    const ModelPose* pose{};
    ModelMaterialState material{};
    ModelInstanceState instance{};
};

std::uint32_t DecodeBoneIndex(std::uint32_t encodedBone);

class RendererModelReference {
public:
    void BeginFrame(const FrameDataStd140& frame);
    void Queue(ModelDrawCommand command);
    void ClearQueue();
    void SortOpaqueConservatively();
    std::size_t QueuedDraws() const { return queue_.size(); }
    const FrameDataStd140& FrameData() const { return frame_; }

private:
    FrameDataStd140 frame_{};
    std::vector<ModelDrawCommand> queue_{};
};

} // namespace reverse_gl46
