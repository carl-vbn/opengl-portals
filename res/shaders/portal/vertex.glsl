#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;

uniform mat4 u_MVP;

out vec2 frag_uv;
out vec4 frag_screenpos;
out vec3 frag_modelpos;

void main()
{
    gl_Position = u_MVP * vec4(aPos.x, aPos.y, aPos.z, 1.0);
    frag_screenpos = gl_Position;
    frag_modelpos = aPos;
    frag_uv = aUV;
}