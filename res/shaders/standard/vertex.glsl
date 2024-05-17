#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 u_M;
uniform mat4 u_MVP;

out vec3 frag_normal;
out vec3 frag_worldpos;

void main()
{
    gl_Position = u_MVP * vec4(aPos, 1.0);
    frag_worldpos = (u_M * vec4(aPos, 1.0)).xyz;

    frag_normal = aNormal;
}