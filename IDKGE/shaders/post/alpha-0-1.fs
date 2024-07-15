#version 460 core

out vec4 fsout_frag_color;
in vec2 fsin_texcoords;

uniform sampler2D un_input;


void main()
{
    vec4 src = texture(un_input, fsin_texcoords);
         src.a = (src.a < 1.0) ? 0.0 : 1.0;

    fsout_frag_color = src;
}

