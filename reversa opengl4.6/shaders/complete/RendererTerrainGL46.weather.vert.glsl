#version 460 core
#define RENDERER_TERRAIN_GL46 1
#define TERRAIN_WEATHER_FX 1

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTextureCoordinate;
layout(location = 2) in vec4 inColor;
layout(std140, binding = 0) uniform FrameData
{
    mat4 view;
    mat4 projection;
    mat4 viewProjection;
    vec4 cameraPosition;
    vec4 viewport;
    vec4 timeFog;
    vec4 tint;
};
out vec2 textureCoordinate;
out vec4 vertexColor;
out float viewDepth;
#if TERRAIN_WEATHER_FX
out vec3 worldPosition;
#endif
void main()
{
    vec4 viewPosition = view * vec4(inPosition, 1.0);
    gl_Position = projection * viewPosition;
    textureCoordinate = inTextureCoordinate;
    vertexColor = inColor;
    // [Genesys][2026-09-06] Distancia REAL ate a camera, nao a profundidade da tela.
    // O horizonte e um cilindro: ele fica a mesma distancia em todas as direcoes. Medindo
    // pela profundidade, um ponto fora do eixo da camera parece mais perto do que esta e
    // recebe menos bruma — com a camera alta e a abertura no maximo, o chao encostava no
    // ceu ainda com um terco de nevoa faltando, e a beirada voltava a aparecer como linha.
    viewDepth = length(inPosition - cameraPosition.xyz);
#if TERRAIN_WEATHER_FX
    worldPosition = inPosition;
#endif
}
