#version 460 core
#extension GL_GOOGLE_include_directive: require

layout (location = 0) out vec4 fsout_frag_color;

in vec2 fsin_texcoord;

flat in vec4  fsin_color;
flat in vec4  fsin_bounds;
flat in float fsin_radius;


void main()
{
    vec2 texel   = fsin_bounds.zw * fsin_texcoord;
    vec2 nearest = clamp(texel, vec2(fsin_radius), fsin_bounds.zw-fsin_radius);

    float alpha = fsin_color.a;
    float dist = distance(texel, nearest);

    // if (dist <= fsin_radius)
    // {
    //     discard;
    // }

    alpha = (dist <= fsin_radius) ? alpha : 0.0;

    fsout_frag_color = vec4(fsin_color.rgb, alpha);
}
