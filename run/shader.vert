#version 460

uniform mat4 MVP;

in vec3 inPos;
in vec3 inNormal;
in float inU;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec3 outColor;

void main()
{
    //gl_Position = MVP * vec4(inPos + inNormal * inU * 0.1, 1.0);
    gl_Position = MVP * vec4(inPos, 1.0);
    outNormal = inNormal;

    outColor = vec3(inU / 1, 0, 1 - inU / 1);
    //outColor = inNormal;
}
