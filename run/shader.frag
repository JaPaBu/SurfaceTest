#version 460

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec3 inColor;

out vec4 outColor;

void main()
{
    //outColor = vec4((inNormal+1)/2, 1.0);
    outColor = vec4(inColor, 1.0);
}