#version 460 core
#define RENDERER_SKY_GL46 1

in vec4 skyTint;
layout(location = 0) out vec4 outputColor;
void main()
{
    // [Genesys][2026-09-06] Ceu chapado: cor unica, sem textura. Ver a nota no RenderSky3.
    outputColor = skyTint;
}
