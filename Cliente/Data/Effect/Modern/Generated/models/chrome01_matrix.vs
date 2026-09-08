// Runtime Matrix4x4 variant of vulkan-main/vulkan-main/Shaders/models/chrome01.vs.
// Keeps the reference material math while using the 4-texel Matrix4x4 skeleton atlas.
// Source contract: HLSL chrome01 + common/SkeletonHelpers.inc with matrix storage.

layout(std140) uniform GlobalConstants
{
    vec4 ShadowColor;
    vec4 ShadowPosition;
    mat4 Shadow;
    mat4 Game3D;
    mat4 Game2D;
    vec4 BillboardQ;
    vec3 LightPosition;
    float WorldTime;
    vec3 ChromeWave;
    float Random;
    vec2 Wave;
    float WaterMove;
    float WindScale;
    float WindSpeed;
    int WaterIndex;
    vec2 Padding1;
} GlobalConstantsData;

uniform sampler2D BonesTexture;

layout(location = 0) in vec3 input_Position;
layout(location = 1) in vec2 input_UV;
layout(location = 2) in vec3 input_Normal;
layout(location = 3) in uvec2 input_Bones;
layout(location = 6) in vec3 input_BodyOrigin;
layout(location = 7) in vec2 input_BodyScale;
layout(location = 8) in vec2 input_Data;
layout(location = 9) in vec4 input_Data2;
layout(location = 10) in vec4 input_BodyLight;
layout(location = 11) in vec4 input_ShadowLight;
layout(location = 12) in uint input_BoneIndex;

layout(location = 0) out vec4 output_Color;
layout(location = 1) out vec2 output_UV;
layout(location = 2) flat out float output_MinAlpha;

vec4 LoadBoneTexel(uint texelIndex)
{
    uint x = texelIndex % 1024u;
    uint y = texelIndex / 1024u;
    return texelFetch(BonesTexture, ivec2(int(x), int(y)), 0);
}

mat4 RequestBone(uint baseBone, uint boneId)
{
    uint texelIndex = (baseBone + boneId) * 4u;
    vec4 row0 = LoadBoneTexel(texelIndex);
    vec4 row1 = LoadBoneTexel(texelIndex + 1u);
    vec4 row2 = LoadBoneTexel(texelIndex + 2u);
    vec4 row3 = LoadBoneTexel(texelIndex + 3u);
    return mat4(row0, row1, row2, row3);
}

void main()
{
    mat4 boneMatrix = RequestBone(input_BoneIndex, input_Bones.x);
    vec4 position = boneMatrix * vec4(input_Position, 1.0);

    if (input_Bones.x != input_Bones.y)
        boneMatrix = RequestBone(input_BoneIndex, input_Bones.y);

    vec3 normal = mat3(boneMatrix[0].xyz,
                       boneMatrix[1].xyz,
                       boneMatrix[2].xyz) * input_Normal;

    position += vec4(normal, 0.0) * input_BodyScale.y;
    position.xyz *= input_BodyScale.x;
    position.xyz += input_BodyOrigin;
    position *= transpose(GlobalConstantsData.Game3D);

    gl_Position = position;
    output_Color = clamp(input_BodyLight, vec4(0.0), vec4(1.0));
    output_UV = normal.zy * 0.5 +
                vec2(GlobalConstantsData.Wave.x,
                     GlobalConstantsData.Wave.x * 2.0);
    output_MinAlpha = input_Data.y;
}