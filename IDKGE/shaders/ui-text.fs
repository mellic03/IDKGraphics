#version 460 core

layout (location = 0) out vec4 fsout_frag_color;


in vec2 fsin_texcoord;
flat in vec3 fsin_extents;
flat in vec4 fsin_color;

uniform sampler2D un_atlas;


void main()
{
    fsout_frag_color = texture(un_atlas, fsin_texcoord);
}
