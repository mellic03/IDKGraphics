#version 460 core

layout (location = 0) out vec4 fsout_frag_color;


in FS_in
{
    vec2 texcoord;
    flat vec3 extents;
    flat vec4 color;
} fsin;

uniform sampler2D un_atlas;


void main()
{
    vec4 src = texture(un_atlas, fsin.texcoord);

    // if (src.a < 0.9)
    // {
    //     discard;
    // }

    fsout_frag_color = src;
}
