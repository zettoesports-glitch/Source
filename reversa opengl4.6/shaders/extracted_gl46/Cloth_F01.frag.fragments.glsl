// EXTRACTED printable GLSL fragments from Main.exe.
// File offsets: 0x00791569..0x00791910
// No invented statements; NUL-separated literals were joined with newlines.
in vec2 textureCoordinate;
in vec4 vertexColor;
in float viewDepth;
uniform sampler2D textureSampler;
uniform int textureEnabled;
uniform int alphaCutoffEnabled;
uniform float alphaReference;
layout(std140, binding = 0) uniform FrameData
    mat4 view;
    mat4 projection;
    mat4 viewProjection;
    vec4 cameraPosition;
    vec4 viewport;
    vec4 timeFog;
    vec4 tint;
};
layout(location = 0) out vec4 outputColor;
void main()
vec4 color = (textureEnabled != 0
? texture(textureSampler, textureCoordinate)
: vec4(1.0)) * vertexColor;
    if (alphaCutoffEnabled != 0 && color.a <= alphaReference)
    {
        discard;
    }
    if (timeFog.w > 0.0)
    {
        float fogRange = max(timeFog.z - timeFog.y, 0.0001);
        float fogFactor = clamp((viewDepth - timeFog.y) / fogRange, 0.0, 1.0);
        color.rgb = mix(color.rgb, tint.rgb, fogFactor * clamp(timeFog.w, 0.0, 1.0));
    }
    outputColor = color;
