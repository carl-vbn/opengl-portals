#version 330 core

uniform vec3 u_lightdir;

in vec3 frag_normal;
out vec4 frag_color;

void main()
{
   frag_color = vec4(1.0, 0.5, 0.2, 1.0) * (dot(frag_normal, u_lightdir) + 0.2);
}