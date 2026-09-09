#version 330 compatibility
out vec2 vUV;
out vec4 vColor;
out vec3 vNormal;

void main() {
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    vUV = gl_MultiTexCoord0.xy;
    vColor = gl_Color;
    vNormal = gl_Normal;
}

