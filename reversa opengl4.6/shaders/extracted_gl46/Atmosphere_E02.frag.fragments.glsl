// EXTRACTED printable GLSL fragments from Main.exe.
// File offsets: 0x0078F4E1..0x0078FA8C
// No invented statements; NUL-separated literals were joined with newlines.
in vec2 textureCoordinate;
uniform vec3 hazeColor;
uniform float hazeOpacity;
uniform float hazeTime;
layout(location = 0) out vec4 outputColor;
float Hash(vec2 position)
    return fract(sin(dot(position, vec2(127.1, 311.7))) * 43758.5453123);
float Noise(vec2 position)
    vec2 cell = floor(position);
    vec2 local = fract(position);
    local = local * local * (3.0 - 2.0 * local);
    float a = Hash(cell);
    float b = Hash(cell + vec2(1.0, 0.0));
    float c = Hash(cell + vec2(0.0, 1.0));
    float d = Hash(cell + vec2(1.0, 1.0));
    return mix(mix(a, b, local.x), mix(c, d, local.x), local.y);
float Fbm(vec2 position)
    float value = 0.0;
    float weight = 0.55;
    for (int octave = 0; octave < 4; ++octave)
    {
        value += Noise(position) * weight;
        position = position * 2.03 + vec2(7.1, 3.7);
        weight *= 0.48;
    }
    return value;
void main()
    vec2 flow = vec2(hazeTime * 0.018, hazeTime * 0.004);
    float mist = Fbm(textureCoordinate * vec2(3.2, 1.8) + flow);
    float wideMist = Noise(textureCoordinate * vec2(1.15, 0.72) - flow * 0.35);
    mist = clamp(mist * 0.72 + wideMist * 0.28, 0.0, 1.0);
    float horizon = 1.0 - clamp(abs(textureCoordinate.y - 0.58) * 1.45, 0.0, 1.0);
    float alpha = hazeOpacity * (0.16 + mist * 0.84) * (0.42 + horizon * 0.58);
    vec3 color = clamp(hazeColor + vec3(0.10, 0.12, 0.16) * mist, 0.0, 1.0);
    outputColor = vec4(color, alpha);
