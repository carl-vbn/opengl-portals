#version 330 core
  
in vec2 frag_uv;
out vec4 frag_color;

uniform sampler2D u_screentex;

void main()
{ 
    frag_color = texture(u_screentex, frag_uv);
}