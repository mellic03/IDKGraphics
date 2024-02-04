#version 460 core
layout (location = 0) in vec3 vsin_pos;

uniform mat4 un_P;
uniform mat4 un_model;

void main()
{
    gl_Position = un_P * un_model * vec4(vsin_pos, 1.0);
}  



