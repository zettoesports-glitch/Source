#version 330 core

layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 UV;
layout(location = 2) in vec3 Normal;
layout(location = 3) in uvec2 Bones;

layout(location = 6) in vec3 BodyOrigin;
layout(location = 7) in vec2 BodyScale;
layout(location = 8) in vec2 Data;
layout(location = 9) in vec4 Data2;
layout(location = 10) in vec4 BodyLight;
layout(location = 11) in vec4 ShadowLight;
layout(location = 12) in uint BoneIndex;

uniform mat4 uProj;
uniform mat4 uView;
uniform sampler2D BonesTexture;

out vec2 vUV;
out vec4 vColor;
out float vMinAlpha;

const uint SKELETON_TEXTURE_WIDTH = 1024u;

vec4 LoadBoneTexel(uint texelIndex)
{
    uint x = texelIndex % SKELETON_TEXTURE_WIDTH;
    uint y = texelIndex / SKELETON_TEXTURE_WIDTH;
    return texelFetch(BonesTexture, ivec2(int(x), int(y)), 0);
}

void RequestBone(uint baseBone, uint boneId, out vec4 rotation, out vec4 positionScale)
{
    // Same contract as SkeletonHelpers.inc in QPS/legacy "dual quaternion" mode:
    // BoneIndex is expressed in bone units and each bone occupies two float4 texels.
    uint texelIndex = (baseBone + boneId) * 2u;
    rotation = LoadBoneTexel(texelIndex + 0u);
    positionScale = LoadBoneTexel(texelIndex + 1u);
}

vec3 RotateQuaternion(vec3 v, vec4 q)
{
    float xx = q.x * q.x;
    float yy = q.y * q.y;
    float zz = q.z * q.z;
    float xy = q.x * q.y;
    float xz = q.x * q.z;
    float yz = q.y * q.z;
    float wx = q.w * q.x;
    float wy = q.w * q.y;
    float wz = q.w * q.z;

    return vec3(
        (1.0 - 2.0 * (yy + zz)) * v.x + (2.0 * (xy - wz)) * v.y + (2.0 * (xz + wy)) * v.z,
        (2.0 * (xy + wz)) * v.x + (1.0 - 2.0 * (xx + zz)) * v.y + (2.0 * (yz - wx)) * v.z,
        (2.0 * (xz - wy)) * v.x + (2.0 * (yz + wx)) * v.y + (1.0 - 2.0 * (xx + yy)) * v.z
    );
}

vec3 TransformPoint(uint boneId, vec3 point)
{
    vec4 q;
    vec4 positionScale;
    RequestBone(BoneIndex, boneId, q, positionScale);
    return RotateQuaternion(point, q) * positionScale.w + positionScale.xyz;
}

vec3 TransformVector(uint boneId, vec3 vectorValue)
{
    vec4 q;
    vec4 positionScale;
    RequestBone(BoneIndex, boneId, q, positionScale);
    return RotateQuaternion(vectorValue, q) * positionScale.w;
}

void main()
{
    vec3 skinnedPosition = TransformPoint(Bones.x, Position);
    vec3 skinnedNormal = TransformVector(Bones.y, Normal);

    // Preserve the reference shader order: optional normal offset, body scale,
    // body origin, then camera matrices supplied by the current MU frontend.
    skinnedPosition += skinnedNormal * BodyScale.y;
    skinnedPosition *= BodyScale.x;
    skinnedPosition += BodyOrigin;

    gl_Position = uProj * uView * vec4(skinnedPosition, 1.0);
    vUV = UV + Data2.xy;
    vColor = BodyLight;
    vMinAlpha = Data.y;
}
