#version 330 core
  
in vec4 frag_position;
in vec2 screen_uv;
out vec4 frag_color;

uniform sampler2D u_rendertex;

void main()
{ 
    frag_color = texture(u_rendertex, frag_position.xy / frag_position.w * 0.5 + 0.5);
}