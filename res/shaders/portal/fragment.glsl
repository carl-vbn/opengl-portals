#version 330 core
  
in vec4 frag_screenpos;
in vec3 frag_modelpos;
in vec3 frag_modelnorm;

out vec4 frag_color;

uniform sampler2D u_rendertex;
uniform vec3 u_color;
uniform float u_outradius;
uniform float u_inradius;

void main()
{
    float t = frag_modelpos.x * frag_modelpos.x + frag_modelpos.y * frag_modelpos.y;
    
    if (t > u_outradius) discard;
    else if (t > u_inradius && frag_modelpos.z > 0) frag_color = vec4(u_color, 1.0);
    else frag_color = texture(u_rendertex, frag_screenpos.xy / frag_screenpos.w * 0.5 + 0.5);
}