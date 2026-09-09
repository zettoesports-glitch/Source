#version 330 compatibility
in vec2 vUV;
in vec4 vColor;
uniform sampler2D uTexture;
uniform float uTime;

void main() {
    vec2 distUV = vUV;
    distUV.x += sin(uTime * 2.0 + vUV.y * 10.0) * 0.005;
    distUV.y += cos(uTime * 2.0 + vUV.x * 10.0) * 0.005;

    vec4 tex = texture(uTexture, distUV);
    gl_FragColor = tex * vColor;
    gl_FragColor.rgb += pow(vColor.r, 5.0) * 0.5;
}

