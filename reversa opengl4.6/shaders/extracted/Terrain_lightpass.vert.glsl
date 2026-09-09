#version 330 core
layout(location = 0) in vec2 inPos;

uniform mat4 uProj;
uniform mat4 uView;
uniform vec2 uTileOffset;
uniform float uTerrainScale;
uniform sampler2D uHeightMap;

out vec2 vUV;

void main()
{
    vec2 mapPos = inPos + uTileOffset;
    float h = texture(uHeightMap, mapPos / 256.0).r;

    vec3 worldPos = vec3(
        mapPos.x * uTerrainScale,
        mapPos.y * uTerrainScale,
        h
    );

    vUV = mapPos / 256.0;
    gl_Position = uProj * uView * vec4(worldPos, 1.0);
}
