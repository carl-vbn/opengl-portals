#version 330 core
  
in vec4 frag_screenpos;
in vec3 frag_modelpos;
in vec2 screen_uv;
out vec4 frag_color;

uniform sampler2D u_rendertex;
uniform vec3 u_color;

void main()
{
    float t = frag_modelpos.x * frag_modelpos.x + frag_modelpos.y * frag_modelpos.y;
    
    if (t > 1 || frag_modelpos.z < 0) discard;
    else if (t > 0.8) frag_color = vec4(u_color, 1.0);
    else frag_color = texture(u_rendertex, frag_screenpos.xy / frag_screenpos.w * 0.5 + 0.5);
}