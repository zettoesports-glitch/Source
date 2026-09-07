#pragma once

#include "../Uniforms/BoneUBO.h"

namespace Render
{
    // Shader contract for the BMD GPU-skinning path.
    // Season 5.2 currently supplies one bone per vertex; the runtime maps it
    // to influence 0 with weight 1.0 and leaves the other influences at zero.
    struct GPUSkinningShaderLayout
    {
        enum
        {
            PositionLocation = 0,
            NormalLocation = 1,
            TexCoordLocation = 2,
            BoneIndexLocation = 3,
            BoneWeightLocation = 4,
            BoneUBOSlot = ::Render::BoneUBOSlot
        };
    };

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
