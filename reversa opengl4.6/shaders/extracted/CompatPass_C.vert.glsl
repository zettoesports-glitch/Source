#version 330 compatibility
out vec2 vUV;
out vec4 vColor;

void main()
{
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    vUV = gl_MultiTexCoord0.xy;
    vColor = min(gl_Color, 1.0);
}

