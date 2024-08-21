#version 460 core

layout (location = 0) out vec4 fsout_frag_color;

in vec2 fsin_texcoords;

uniform sampler2D un_color0;
uniform sampler2D un_depth0;

uniform sampler2D un_color1;
uniform sampler2D un_depth1;


void main()
{
    const int KERNEL_HW = 4;

    vec4 color0 = textureLod(un_color0, fsin_texcoords, 0.0);
    vec4 color1 = textureLod(un_color1, fsin_texcoords, 0.0);

    float depth0 = textureLod(un_depth0, fsin_texcoords, 0.0).r;
    float depth1 = textureLod(un_depth1, fsin_texcoords, 0.0).r;

    fsout_frag_color = (depth0 < depth1) ? color0 : color1;
}