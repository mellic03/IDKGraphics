#version 460 core

layout (location = 0) out vec4 fsout_frag_color;

in vec2 fsin_texcoords;

uniform sampler2D un_input;


void main()
{
    fsout_frag_color = textureLod(un_input, fsin_texcoords, 0.0);
}