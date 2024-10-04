#version 460 core

layout (location = 0) out vec4 fsout_frag_color;


in FS_in
{
    vec2 texcoord;
    flat vec3 extents;
    flat vec4 color;
} fsin;


void main()
{
    vec3  color  = fsin.color.rgb;

    float width  = fsin.extents[0];
    float height = fsin.extents[1];
    float radius = fsin.extents[2];

    ivec2 texel   = ivec2(fsin.extents.xy * fsin.texcoord);
    vec2  nearest = clamp(texel, vec2(radius), fsin.extents.xy - radius);
    float dist    = distance(texel, nearest);
    float alpha   = 1.0 - clamp(dist - radius, 0.0, 1.0);

    // if (dist > radius)
    // {
    //     discard;
    // }

    fsout_frag_color = vec4(color, alpha*fsin.color.a);
}

