#version 330 compatibility
in vec2 vUV;
in vec4 vColor;
uniform sampler2D uTexture;
uniform int uRenderType;
out vec4 FragColor;

void main()
{
    vec4 tex = texture(uTexture, vUV);
    vec4 color = tex * vColor;

    if (uRenderType == 1)
    {
        if (color.a < 0.2) discard;
    }

    if (uRenderType == 2)
    {
        color.rgb *= color.a;
    }

    if (uRenderType == 3)
    {
        color.rgb *= 1.5;
    }

    gl_FragColor = color;
}

