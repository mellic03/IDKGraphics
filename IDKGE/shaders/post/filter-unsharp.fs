#version 460 core

layout (location = 0) out vec4 fsout_frag_color;

in vec2 fsin_texcoords;
uniform sampler2D un_input;


vec4 unsharp( float scale )
{
    vec2  tsize  = textureSize(un_input, 0);
    ivec2 texel  = ivec2(tsize * fsin_texcoords);
    vec4  result = vec4(0.0);

    result += (4.0*scale + 1.0) * texelFetch(un_input, texel + 4*ivec2(0,  0), 0);
    result += -scale * texelFetch(un_input, texel + 4*ivec2(0, -1), 0);
    result += -scale * texelFetch(un_input, texel + 4*ivec2(0, +1), 0);
    result += -scale * texelFetch(un_input, texel + 4*ivec2(-1, 0), 0);
    result += -scale * texelFetch(un_input, texel + 4*ivec2(+1, 0), 0);

    return result;
}



void main()
{
    vec2  tsize  = textureSize(un_input, 0);
    ivec2 texel  = ivec2(tsize * fsin_texcoords);

    // fsout_frag_color = unsharp(1.0);

    fsout_frag_color = texture(un_input, fsin_texcoords);
}

