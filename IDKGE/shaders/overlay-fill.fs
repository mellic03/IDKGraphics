#version 460 core

#extension GL_GOOGLE_include_directive: require
#include "./include/storage.glsl"

out vec4 fsout_frag_color;
in vec2 fsin_texcoords;

uniform float un_alpha;
uniform vec3  un_color;

void main()
{
    fsout_frag_color = vec4(un_color, un_alpha);
}
