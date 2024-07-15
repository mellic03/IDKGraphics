#version 460 core

layout (location = 0) out vec4 fsout_frag_color;

in vec2 fsin_texcoords;
uniform sampler2D un_input;

void main()
{
    float ao = texture(un_input, fsin_texcoords).r;
    fsout_frag_color = vec4(vec3(ao), 1.0);
}
