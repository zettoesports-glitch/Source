// RECONSTRUÇÃO E MAPA do RendererModel.cpp a partir de evidência literal do Main.exe.
// Os shaders exatos estão em ../shaders/extracted/RendererModel_*.glsl.
// Este arquivo descreve a arquitetura confirmada, não pretende ser o translation unit original.
#include <array>
#include <cstdint>

namespace reverse_gl46 {

struct Mat4 { float m[16]; };
struct Vec4 { float x,y,z,w; };

struct FrameDataStd140 {
    Mat4 frameView;
    Mat4 frameProjection;
    Mat4 frameViewProjection;
    Vec4 frameCameraPosition;
    Vec4 frameViewport;
    Vec4 frameTimeFog;
    Vec4 frameTint;
};

struct ModelVertex {
    float position[3];
    float normal[3];
    float texcoord[2];
    std::uint32_t bone;
};

struct ModelMaterialState {
    int textureEnabled{};
    int alphaCutoffEnabled{};
    float alphaReference{};
    int chromeMode{};
    float wave1{};
    float wave2{};
    float chromeL[3]{};
    float blendMeshTexCoord[2]{};
    int fogEnabled{};
    float fogColor[3]{};
    float fogStart{};
    float fogEnd{};
    float fogIntensity{};
};

struct ModelPose {
    // Literalmente confirmado pelo shader: u_Bones[200].
    std::array<Mat4, 200> bones{};
};

// Fases registradas pelo binário:
// G01ModelExplicitMatrices   -> matrizes explícitas / FrameData.
// G02ModelMaterialState      -> commit explícito de material/legacy state.
// G03ModelBoneScratch        -> scratch de bones reutilizável.
// G04ModelPoseReuse          -> reutilização/cache de pose.
// G06ModelQueueObserve       -> command queue/observação de draws.
// G07ModelOpaqueSort         -> ordenação conservadora de opacos.
// H08ModelStateCommit        -> cache/commit de estado.
// H08ModelInstanceUniformCache -> evita uploads repetidos por instância.

// Achado literal importante:
// full_pose_ubo_selected=0
// full_pose_ubo_rejection_reason=measured_sparse_mesh_bone_sets
// Portanto o FrameData usa UBO, mas o shader de model mantém u_Bones[200] como uniform array.

std::uint32_t DecodeBoneIndex(std::uint32_t encodedBone)
{
    // Exatamente como no GLSL recuperado.
    return encodedBone / 3u;
}

} // namespace reverse_gl46
