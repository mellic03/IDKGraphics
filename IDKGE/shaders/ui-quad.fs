#version 460 core

layout (location = 0) out vec4 fsout_frag_color;

in vec2 fsin_texcoord;
flat in vec3 fsin_extents;
flat in vec4 fsin_color;


void main()
{
    vec3  color  = fsin_color.rgb;

    float width  = fsin_extents[0];
    float height = fsin_extents[1];
    float radius = fsin_extents[2];

    ivec2 texel   = ivec2(fsin_extents.xy * fsin_texcoord);
    vec2  nearest = clamp(texel, vec2(radius), fsin_extents.xy - radius);
    float dist    = distance(texel, nearest);
    float alpha   = 1.0 - clamp(dist - radius, 0.0, 1.0);

    // if (dist > radius)
    // {
    //     discard;
    // }

    fsout_frag_color = vec4(color, alpha*fsin_color.a);
}

