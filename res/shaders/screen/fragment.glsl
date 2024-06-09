#version 330 core
  
in vec2 frag_uv;
out vec4 frag_color;

uniform sampler2D u_screentex;
uniform float u_aspectratio;

void main()
{
    frag_color = texture(u_screentex, frag_uv);

    // Crosshair
    if (length((frag_uv - vec2(0.5)) * vec2(u_aspectratio, 1.0)) < 0.002) {
        frag_color.rgb = vec3(1.0) - frag_color.rgb;
    }

    // Gamma correction
    frag_color.rgb = pow(frag_color.rgb, vec3(0.454545455));
}