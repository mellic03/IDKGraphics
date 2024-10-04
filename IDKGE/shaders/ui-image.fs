#version 460 core

layout (location = 0) out vec4 fsout_frag_color;


in FS_in
{
    vec2 texcoord;
    flat vec3 extents;
    flat vec4 color;
} fsin;

uniform sampler2D un_texture;


void main()
{
    vec2  tsize   = textureSize(un_texture, 0);
    float taspect = tsize.x / tsize.y;

    vec2  qsize   = vec2(fsin.extents[0], fsin.extents[1]);
    float qaspect = qsize.x / qsize.y;


    vec2 uv;

    if (qaspect > taspect)
    {
        uv = fsin.texcoord * vec2(qaspect, 1.0);
    }

    else
    {
        uv = fsin.texcoord * vec2(1.0, qaspect);
    }


    vec4 src = texture(un_texture, uv);

    // if (src.a < 0.9)
    // {
    //     discard;
    // }

    fsout_frag_color = src;
}
