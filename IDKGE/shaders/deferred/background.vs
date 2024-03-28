#version 460 core
#extension GL_GOOGLE_include_directive: require

layout (location = 0) in vec3 vsin_pos;
layout (location = 1) in vec2 vsin_texcoords;

#include "../include/UBOs.glsl"

out vec4 fsin_fragpos;

uniform mat4 un_model;

void main()
{
    vec4 worldpos = inverse(un_view) * un_model * vec4(vsin_pos, 1.0);
    fsin_fragpos = worldpos;
    gl_Position = un_projection * un_view * worldpos;
}
