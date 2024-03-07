#version 460 core
#extension GL_GOOGLE_include_directive: require

#include "../UBOs/UBOs.glsl"
#include "../include/util.glsl"


layout (location = 0) out vec4 fsout_frag_color;

in vec2 fsin_texcoords;
uniform sampler2D un_texture_0;


float IDK_ComputeLuminance( vec3 color )
{
    return dot(color, vec3(0.2126, 0.7152, 0.0722));
}


void main()
{
    vec3 color = texture(un_texture_0, fsin_texcoords).rgb;

    float luminance = IDK_ComputeLuminance(color);
    color *= luminance;

    fsout_frag_color = vec4(color, 1.0);
}
