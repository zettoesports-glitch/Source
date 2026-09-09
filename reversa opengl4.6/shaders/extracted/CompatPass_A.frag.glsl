#version 330 compatibility
in vec2 vUV;
in vec4 vColor;
uniform sampler2D uTexture;
uniform float uWaterMove;

void main()
{
    vec4 tex = texture(uTexture, vUV);

    if (tex.a < 0.1) discard;

    gl_FragColor = tex * vColor;
}

