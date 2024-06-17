#version 460 core
#extension GL_GOOGLE_include_directive: require

#include "./include/UBOs.glsl"

layout (location = 0) out vec4 fsout_frag_color;

in vec2 fsin_texcoord;

uniform sampler2D un_atlas;


void main()
{
    vec4 src = texture(un_atlas, fsin_texcoord);
    fsout_frag_color = vec4(src.rgb, src.a);
}
