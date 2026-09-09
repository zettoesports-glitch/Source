#version 330 core
in vec2 vUV;

uniform vec3 uColor;
uniform float uOpacity;
uniform float uTime;

out vec4 FragColor;

float Hash(vec2 p)
{
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123);
}

float Noise(vec2 p)
{
    vec2 cell = floor(p);
    vec2 local = fract(p);
    local = local * local * (3.0 - 2.0 * local);
    float a = Hash(cell);
    float b = Hash(cell + vec2(1.0, 0.0));
    float c = Hash(cell + vec2(0.0, 1.0));
    float d = Hash(cell + vec2(1.0, 1.0));
    return mix(mix(a, b, local.x), mix(c, d, local.x), local.y);
}

float Fbm(vec2 p)
{
    float value = 0.0;
    float weight = 0.55;
    for (int octave = 0; octave < 4; ++octave)
    {
        value += Noise(p) * weight;
        p = p * 2.03 + vec2(7.1, 3.7);
        weight *= 0.48;
    }
    return value;
}

void main()
{
    vec2 flow = vec2(uTime * 0.018, uTime * 0.004);
    float mist = Fbm(vUV * vec2(3.2, 1.8) + flow);
    float wideMist = Noise(vUV * vec2(1.15, 0.72) - flow * 0.35);
    mist = clamp(mist * 0.72 + wideMist * 0.28, 0.0, 1.0);

    float horizon = 1.0 - clamp(abs(vUV.y - 0.58) * 1.45, 0.0, 1.0);
    float alpha = uOpacity * (0.16 + mist * 0.84) * (0.42 + horizon * 0.58);
    vec3 mistColor = clamp(uColor + vec3(0.10, 0.12, 0.16) * mist, 0.0, 1.0);
    FragColor = vec4(mistColor, alpha);
}
