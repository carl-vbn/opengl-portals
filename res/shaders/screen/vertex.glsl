#version 330 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUV;

out vec2 frag_uv;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0); 
    frag_uv = aUV;
}  