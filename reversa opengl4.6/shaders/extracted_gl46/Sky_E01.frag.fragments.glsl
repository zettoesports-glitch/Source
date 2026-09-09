// EXTRACTED printable GLSL fragments from Main.exe.
// File offsets: 0x0078F169..0x0078F230
// No invented statements; NUL-separated literals were joined with newlines.
in vec4 skyTint;
layout(location = 0) out vec4 outputColor;
void main()
    // [Genesys][2026-09-06] Ceu chapado: cor unica, sem textura. Ver a nota no RenderSky3.
    outputColor = skyTint;
