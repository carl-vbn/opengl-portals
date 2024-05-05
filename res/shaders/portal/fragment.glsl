#version 330 core
  
in vec2 frag_uv;
out vec4 frag_color;

uniform sampler2D u_rendertex;

void main()
{ 
    frag_color = texture(u_rendertex, frag_uv);
}