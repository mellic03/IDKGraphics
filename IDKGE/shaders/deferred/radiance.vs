#version 460 core

out vec2 fsin_texcoord;

void main()
{
    const vec2 position = vec2(gl_VertexID % 2, gl_VertexID / 2) * 4.0 - 1;
    const vec2 texcoord = (position + 1) * 0.5;

    fsin_texcoord = texcoord;
    gl_Position = vec4(position, 1.0, 1.0);
}
