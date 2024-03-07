#version 460 core

out vec4 fsout_frag_color;

in vec2 fsin_texcoords;
uniform sampler2D un_texture_0;

uniform vec2 un_r_offset;
uniform vec2 un_g_offset;
uniform vec2 un_b_offset;


void main()
{
    vec2 center = fsin_texcoords + vec2(0.5, 0.5);
    float d = distance(fsin_texcoords, vec2(0.5, 0.5));
    float strength = d*d * un_camera.aberration_b.z;


    vec2 r_offset  = strength * un_camera.aberration_rg.xy;
    vec2 g_offset  = strength * un_camera.aberration_rg.zw;
    vec2 b_offset  = strength * un_camera.aberration_b.xy;

    float r = texture( un_texture_0, fsin_texcoords - g_offset - b_offset ).r;
    float g = texture( un_texture_0, fsin_texcoords - r_offset - b_offset ).g;
    float b = texture( un_texture_0, fsin_texcoords - r_offset - g_offset ).b;

    vec3 color = vec3(r, g, b);

    fsout_frag_color = vec4(color, 1.0);
}
