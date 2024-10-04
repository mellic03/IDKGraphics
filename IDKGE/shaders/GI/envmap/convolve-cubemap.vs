#version 460 core

layout (location = 0) in vec3 vsin_pos;

out vec3 fsin_fragpos;

uniform mat4 un_PV;

void main()
{
    fsin_fragpos = vsin_pos;
    gl_Position = un_PV * vec4(fsin_fragpos, 1.0);
}
