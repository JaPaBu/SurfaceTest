#version 460

uniform mat4 uMv;
uniform mat4 uP;

in vec3 inPos;
in vec3 inNormal;
in float inU;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec3 outColor;
layout(location = 2) out vec3 outPos;

void main()
{
    float uN = clamp(inU / 10, -1, 1);
    //uN = 0;
    float uC = clamp(inU / 1, -1, 1) / 2 + 0.5;

    vec4 pos = uMv * vec4(inPos + inNormal * uN, 1.0);
    gl_Position = uP * pos;

    outNormal = normalize(transpose(inverse(mat3(uMv))) * inNormal);
    outColor = vec3(uC / 1, 0, 1 - uC / 1);
    outPos = pos.xyz;
}
