#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNorm;
layout(location = 2) in vec2 aTex;
layout(location = 3) in uint aBone;

layout(std140) uniform FrameData
{
    mat4 frameView;
    mat4 frameProjection;
    mat4 frameViewProjection;
    vec4 frameCameraPosition;
    vec4 frameViewport;
    vec4 frameTimeFog;
    vec4 frameTint;
};

uniform mat4 u_Bones[200];
uniform vec3 u_BodyOrigin;

// [Genesys][2026-09-08] Para onde o sol joga a sombra.
// xy = deslocamento nos dois eixos, z = altura do sol.
// Sem ciclo de dia e noite o cliente manda (2000, 0, 4000), que e o valor fixo
// de sempre, e ai o desenho sai identico ao de antes.
uniform vec3 u_SunOffset;

void main()
{
    int idx = int(aBone) / 3;
    mat4 bone = u_Bones[idx];
    vec4 worldPos = bone * vec4(aPos, 1.0);

    vec3 p = worldPos.xyz - u_BodyOrigin;
    float d = p.z - u_SunOffset.z;
    p.x += p.z * (p.x + u_SunOffset.x) / d;
    p.y += p.z * (p.y + u_SunOffset.y) / d;
    p.z = 0.5;
    worldPos.xyz = p + u_BodyOrigin;

    gl_Position = frameProjection * frameView * worldPos;
}
