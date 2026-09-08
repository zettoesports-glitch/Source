// Matrix4x4 ModernBMD shadow-texture variant.
// Shadow projection matches Cliente/Data/Effect/VBO/Model.vs when u_shadowMode is enabled,
// while skeleton/instance decoding follows the generated texture_matrix.vs contract.

struct VertexInput
{
    vec3 Position;
    vec2 UV;
    vec3 Normal;
    uvec2 Bones;
    vec3 BodyOrigin;
    vec2 BodyScale;
    vec2 Data;
    vec4 Data2;
    vec4 BodyLight;
    vec4 ShadowLight;
    uint BoneIndex;
};

struct PixelInput
{
    vec4 Position;
    vec4 Color;
    vec2 UV;
    float MinAlpha;
};

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

uniform highp sampler2D BonesTexture;

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

highp mat4 spvWorkaroundRowMajor(highp mat4 wrap) { return transpose(wrap); }
mediump mat4 spvWorkaroundRowMajorMP(mediump mat4 wrap) { return transpose(wrap); }

vec4 LoadBoneTexel(uint texelIndex)
{
    uint boneX = texelIndex % 1024u;
    uint boneY = texelIndex / 1024u;
    return texelFetch(BonesTexture, ivec3(uvec3(boneX, boneY, 0u)).xy, ivec3(uvec3(boneX, boneY, 0u)).z);
}

mat4 RequestBone(inout uint boneIndex, uint boneId)
{
    boneIndex += boneId;
    boneIndex *= 4u;
    uint param = boneIndex;
    vec4 row0 = LoadBoneTexel(param);
    vec4 row1 = LoadBoneTexel(param + 1u);
    vec4 row2 = LoadBoneTexel(param + 2u);
    vec4 row3 = LoadBoneTexel(param + 3u);
    return mat4(row0, row1, row2, row3);
}

PixelInput _main(VertexInput _input)
{
    uint param = _input.BoneIndex;
    uint param_1 = _input.Bones.x;
    mat4 BoneMatrix = RequestBone(param, param_1);

    PixelInput _output;
    _output.Position = BoneMatrix * vec4(_input.Position, 1.0);

    if (_input.Bones.x != _input.Bones.y)
    {
        uint param_2 = _input.BoneIndex;
        uint param_3 = _input.Bones.y;
        BoneMatrix = RequestBone(param_2, param_3);
    }

    vec3 normal = normalize(mat3(BoneMatrix[0].xyz, BoneMatrix[1].xyz, BoneMatrix[2].xyz) * _input.Normal);
    _output.Position += vec4(normal, 0.0) * _input.BodyScale.y;
    _output.Position.xyz *= _input.BodyScale.x;
    _output.Position.xyz += _input.BodyOrigin;

    // Exact legacy VBO shadowMode projection from Data/Effect/VBO/Model.vs.
    // The captured immutable body origin is used instead of the mutable BMD field.
    vec3 p = _output.Position.xyz - _input.BodyOrigin;
    const float sx = 2000.0;
    const float sy = 4000.0;
    p.x += p.z * (p.x + sx) / (p.z - sy);
    p.z = 0.5;
    _output.Position.xyz = p + _input.BodyOrigin;

    _output.Position *= spvWorkaroundRowMajor(GlobalConstantsData.Game3D);
    _output.UV = _input.UV + _input.Data2.xy;
    _output.MinAlpha = _input.Data.y;

    if (_input.Data.x == 1.0)
    {
        float lightBlend = clamp(_input.BodyLight.w, 0.0, 1.0);
        float Luminosity = (((dot(normal, GlobalConstantsData.LightPosition) * 0.8) + 0.4) * lightBlend) + (1.0 - lightBlend);
        if (Luminosity < 0.2)
            Luminosity = 0.2;
        _output.Color = clamp(_input.BodyLight * vec4(Luminosity, Luminosity, Luminosity, 1.0), vec4(0.0), vec4(1.0));
    }
    else
    {
        _output.Color = clamp(_input.BodyLight, vec4(0.0), vec4(1.0));
    }

    return _output;
}

void main()
{
    VertexInput _input;
    _input.Position = input_Position;
    _input.UV = input_UV;
    _input.Normal = input_Normal;
    _input.Bones = input_Bones;
    _input.BodyOrigin = input_BodyOrigin;
    _input.BodyScale = input_BodyScale;
    _input.Data = input_Data;
    _input.Data2 = input_Data2;
    _input.BodyLight = input_BodyLight;
    _input.ShadowLight = input_ShadowLight;
    _input.BoneIndex = input_BoneIndex;

    PixelInput flattenTemp = _main(_input);
    gl_Position = flattenTemp.Position;
    output_Color = flattenTemp.Color;
    output_UV = flattenTemp.UV;
    output_MinAlpha = flattenTemp.MinAlpha;
}
