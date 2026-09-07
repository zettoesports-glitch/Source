#ifndef BMD_GPU_SKINNING_SHADER_CONTRACT_GLSL
#define BMD_GPU_SKINNING_SHADER_CONTRACT_GLSL

// Phase 7 GPU skinning contract.
// VAO locations:
//   0 = vec3 position
//   1 = vec3 normal
//   2 = vec2 texcoord
//   3 = uvec4 bone indices
//   4 = vec4 bone weights

layout(location = 3) in uvec4 aBoneIndices;
layout(location = 4) in vec4  aBoneWeights;

layout(std140, binding = 2) uniform BoneBlock
{
    mat4 uBoneMatrices[200];
};

mat4 BMD_BuildSkinMatrix()
{
    return
        uBoneMatrices[aBoneIndices.x] * aBoneWeights.x +
        uBoneMatrices[aBoneIndices.y] * aBoneWeights.y +
        uBoneMatrices[aBoneIndices.z] * aBoneWeights.z +
        uBoneMatrices[aBoneIndices.w] * aBoneWeights.w;
}

vec4 BMD_SkinPosition(vec3 position)
{
    return BMD_BuildSkinMatrix() * vec4(position, 1.0);
}

vec3 BMD_SkinNormal(vec3 normal)
{
    return mat3(BMD_BuildSkinMatrix()) * normal;
}

#endif
