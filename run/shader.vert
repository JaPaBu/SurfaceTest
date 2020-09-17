#version 460

uniform mat4 MVP;

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
    float uC = clamp(inU / 1, 0, 1);

    gl_Position = MVP * vec4(inPos + inNormal * uN, 1.0);
    //gl_Position = MVP * vec4(inPos, 1.0);
    outNormal = inNormal;

    outColor = vec3(uC / 1, 0, 1 - uC / 1);
    //outColor = inNormal;

    outPos = inPos;
}
