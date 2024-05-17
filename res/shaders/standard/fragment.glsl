#version 330 core

uniform vec3 u_lightdir;
uniform vec3 u_color;
uniform bool u_highlightfrontface;
uniform vec3 u_slicenormal;
uniform vec3 u_slicepos;

in vec3 frag_normal;
in vec3 frag_worldpos;
out vec4 frag_color;

bool is_zero(vec3 vector) {
   return vector.x == 0 && vector.y == 0 && vector.z == 0;
}

void main()
{
   if (!is_zero(u_slicenormal) && dot(frag_worldpos - u_slicepos, u_slicenormal) < 0) {
      discard;
   }

   frag_color = (u_highlightfrontface && length(frag_normal - vec3(0.0, 0.0, -1.0)) < 0.001) ? vec4(1.0, 0.0, 1.0, 1.0) : vec4(u_color, 1.0) * clamp(dot(frag_normal, -u_lightdir), 0.1, 1.0);
}