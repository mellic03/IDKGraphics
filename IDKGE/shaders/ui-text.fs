#version 460 core

layout (location = 0) out vec4 fsout_frag_color;


in vec2 fsin_texcoord;
flat in vec3 fsin_extents;
flat in vec4 fsin_color;

uniform sampler2D un_atlas;


void main()
{
    vec4 src = texture(un_atlas, fsin_texcoord);

    // if (src.a < 0.9)
    // {
    //     discard;
    // }

    fsout_frag_color = src;
}
