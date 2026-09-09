#version 460 core
#define RENDERER2D_GL46 1

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTextureCoordinate;
layout(location = 2) in vec4 inColor;
layout(std140, binding = 0) uniform FrameData
{
    mat4 view;
    mat4 projection;
    mat4 viewProjection;
    vec4 cameraPosition;
    vec4 viewport;
    vec4 timeFog;
    vec4 tint;
};
out vec2 textureCoordinate;
out vec4 vertexColor;
void main()
{
    gl_Position = viewProjection * vec4(inPosition, 1.0);
    textureCoordinate = inTextureCoordinate;
    vertexColor = inColor;
}
