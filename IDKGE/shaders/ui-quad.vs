#version 460 core

layout (location = 0) in vec3 vsin_pos;
layout (location = 1) in vec2 vsin_texcoord;
layout (location = 2) in vec3 vsin_extents;
layout (location = 3) in vec4 vsin_color;


out vec2 fsin_texcoord;
flat out vec3 fsin_extents;
flat out vec4 fsin_color;

uniform mat4 un_projection;


void main()
{
    fsin_texcoord = vsin_texcoord.xy;
    fsin_extents  = vsin_extents;
    fsin_color    = vsin_color;

    gl_Position = un_projection * vec4(vsin_pos, 1.0);
}

