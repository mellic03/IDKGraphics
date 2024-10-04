#version 460 core

layout (location = 0) out vec4 fsout_frag_color;

in vec2 fsin_texcoords;

uniform sampler2D un_foliage_color;
uniform sampler2D un_foliage_depth;
uniform sampler2D un_gbuffer_depth;


void main()
{
    vec4  color  = textureLod(un_foliage_color, fsin_texcoords, 0.0);
    float depth0 = textureLod(un_foliage_depth, fsin_texcoords, 0.0).r;
    float depth1 = textureLod(un_gbuffer_depth, fsin_texcoords, 0.0).r;

    if (depth0 > depth1)
    {
        color.a = 0.0;
    }

    fsout_frag_color = color;
}