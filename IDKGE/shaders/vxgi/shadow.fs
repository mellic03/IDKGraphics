#version 460 core

layout (location = 0) out vec4 dummy;
// layout (location = 1) out vec4 color;

void main()
{
    // gl_FragDepth = gl_FragCoord.z;
    // gl_FragDepth += gl_FrontFacing ? BIAS : 0.0;

    // color = vec4(vec3(gl_FragCoord.z), 1.0);
}
