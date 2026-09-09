#version 460 core
#define RENDERER2D_GL46 1

in vec2 textureCoordinate;
in vec4 vertexColor;
uniform sampler2D textureSampler;
uniform int textureEnabled;
uniform int textureCombineMode;
uniform int textureSampleMode;
uniform int alphaCutoffEnabled;
layout(location = 0) out vec4 outputColor;
void main()
{
    vec4 sampled = textureEnabled != 0 ? texture(textureSampler, textureCoordinate) : vec4(1.0);
    if (textureEnabled != 0 && textureSampleMode == 1)
    {
        sampled = vec4(1.0, 1.0, 1.0, sampled.r);
    }
    // The compatibility rasterizer clamps the interpolated primary color
    // before applying the legacy texture environment.  Some login scenes
    // intentionally animate Color4f above 1.0 (the MU logo reaches 10.0), so
    // using the raw value here washes the textured detail out to white.
    vec4 primaryColor = clamp(vertexColor, 0.0, 1.0);
    vec4 combined = sampled * primaryColor;
    if (textureEnabled != 0 && textureCombineMode == 1)
    {
        combined = vec4(clamp(sampled.rgb + primaryColor.rgb, 0.0, 1.0), sampled.a * primaryColor.a);
    }
    if (alphaCutoffEnabled != 0 && combined.a <= 0.25)
    {
        discard;
    }
    outputColor = combined;
}
