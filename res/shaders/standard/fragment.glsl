#version 330 core

uniform vec3 u_lightdir;
uniform vec3 u_color;
uniform bool u_highlightfrontface;

in vec3 frag_normal;
out vec4 frag_color;

void main()
{
   frag_color = (u_highlightfrontface && length(frag_normal - vec3(0.0, 0.0, -1.0)) < 0.001) ? vec4(1.0, 0.0, 1.0, 1.0) : vec4(u_color, 1.0) * clamp(dot(frag_normal, -u_lightdir), 0.1, 1.0);
}