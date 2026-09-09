#version 330 core
in vec2 vUV;
uniform sampler2D uLightMap;

out vec4 FragColor;

void main()
{
    vec3 light = texture(uLightMap, vUV).rgb;
    FragColor = vec4(light, 1.0);
}
