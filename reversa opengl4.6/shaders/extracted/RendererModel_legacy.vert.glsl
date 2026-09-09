#version 330
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNorm;
layout(location = 2) in vec2 aTex;
layout(location = 3) in uint aBone;

uniform mat4 uProj;
uniform mat4 uView;
uniform mat4 u_Bones[200];
uniform vec4 u_BodyLight;
uniform vec4 u_LightPosition;
uniform bool u_enableLight;

out vec4 v_color;
out vec3 v_normal;
out vec2 v_uv;
out float v_viewDistance;

void main()
{
    int idx = int(aBone) / 3;
    mat4 bone = u_Bones[idx];
    vec4 worldPos = bone * vec4(aPos, 1.0);
    vec4 viewPos = uView * worldPos;
    gl_Position = uProj * viewPos;
    v_viewDistance = length(viewPos.xyz);

    vec3 normal = normalize(mat3(bone) * aNorm);
    v_normal = normal;

    vec4 final_color;

    if (u_enableLight)
    {
        vec3 lightDir = u_LightPosition.xyz;
        float Luminosity = dot(v_normal, lightDir) * 0.8 + 0.4;
        Luminosity = max(Luminosity, 0.2);
        final_color = vec4(u_BodyLight.rgb * Luminosity, u_BodyLight.a);
    }
    else
    {
        final_color = u_BodyLight;
    }

    v_color = min(final_color, 1.0);
    v_uv = aTex;
}
