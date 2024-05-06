#version 330 core

uniform vec3 u_lightdir;
uniform vec3 u_color;

in vec3 frag_normal;
out vec4 frag_color;

void main()
{
   frag_color = vec4(u_color, 1.0) * clamp(dot(frag_normal, -u_lightdir), 0.1, 1.0);
}