#pragma once

namespace Render
{
    // Shader contract for the future BMD GPU-skinning path.
    // The current BMD asset format exposes one bone per vertex, so the
    // runtime may use weight0=1 and weights1..3=0 without changing assets.
    struct GPUSkinningShaderLayout
    {
        enum
        {
            PositionLocation = 0,
            NormalLocation = 1,
            TexCoordLocation = 2,
            BoneIndexLocation = 3,
            BoneWeightLocation = 4,
            BoneUBOSlot = 2
        };
    };

    // Keep this shader source backend-neutral until the real VBO shader
    // assets are migrated. Normals are skinned with the same weighted matrix
    // as positions; translation is removed from the normal transform.
    static const char* const GPUSkinningVertexShader =
        "#version 330 core\\n"
        "layout(location=0) in vec3 a_Position;\\n"
        "layout(location=1) in vec3 a_Normal;\\n"
        "layout(location=2) in vec2 a_TexCoord;\\n"
        "layout(location=3) in uvec4 a_BoneIndex;\\n"
        "layout(location=4) in vec4 a_BoneWeight;\\n"
        "layout(std140, binding=2) uniform BoneBlock { mat4 u_Bones[200]; };\\n"
        "uniform mat4 u_Model;\\n"
        "uniform mat4 u_ViewProjection;\\n"
        "out vec2 v_TexCoord;\\n"
        "out vec3 v_Normal;\\n"
        "void main() {\\n"
        "    mat4 skin = a_BoneWeight.x * u_Bones[a_BoneIndex.x] +\\n"
        "                a_BoneWeight.y * u_Bones[a_BoneIndex.y] +\\n"
        "                a_BoneWeight.z * u_Bones[a_BoneIndex.z] +\\n"
        "                a_BoneWeight.w * u_Bones[a_BoneIndex.w];\\n"
        "    vec4 skinnedPosition = skin * vec4(a_Position, 1.0);\\n"
        "    vec3 skinnedNormal = mat3(skin) * a_Normal;\\n"
        "    v_Normal = normalize(mat3(u_Model) * skinnedNormal);\\n"
        "    v_TexCoord = a_TexCoord;\\n"
        "    gl_Position = u_ViewProjection * u_Model * skinnedPosition;\\n"
        "}\\n";
}
