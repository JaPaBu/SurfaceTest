#version 460

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inPos;

out vec4 outColor;

const vec3 LIGHT_POS = vec3(10, 10, 10);

void main()
{
    //outColor = vec4((inNormal+1)/2, 1.0);
    //outColor = vec4(inColor * max(0.1, dot(inNormal, LIGHT_POS - inPos)), 1.0);
    outColor = vec4(inColor, 1.0);
}