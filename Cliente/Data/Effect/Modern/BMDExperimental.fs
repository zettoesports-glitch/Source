#version 330 core

in vec2 vUV;
in vec4 vColor;
in float vMinAlpha;

uniform sampler2D Tex1;

out vec4 FragColor;

void main()
{
    vec4 color = texture(Tex1, vUV) * vColor;
    if (vMinAlpha > 0.0 && color.a < vMinAlpha)
        discard;
    FragColor = color;
}
