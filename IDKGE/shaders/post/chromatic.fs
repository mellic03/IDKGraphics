#version 460 core
#extension GL_ARB_bindless_texture: require
#extension GL_GOOGLE_include_directive: require

#include "../include/storage.glsl"
#include "../include/util.glsl"
#include "../include/pbr.glsl"

out vec4 fsout_frag_color;

in vec2 fsin_texcoords;
uniform sampler2D un_input;


void main()
{
    IDK_Camera camera = IDK_RenderData_GetCamera();

    // vec2 center = fsin_texcoords + vec2(0.5, 0.5);
    // float d = distance(fsin_texcoords, vec2(0.5, 0.5));
    // float strength = d*d * un_camera.aberration_b.z;

    vec2 r_offset  = camera.chromatic_strength.r * camera.chromatic_r;
    vec2 g_offset  = camera.chromatic_strength.r * camera.chromatic_r;
    vec2 b_offset  = camera.chromatic_strength.r * camera.chromatic_r;

    float r = texture( un_input, fsin_texcoords - g_offset - b_offset ).r;
    float g = texture( un_input, fsin_texcoords - r_offset - b_offset ).g;
    float b = texture( un_input, fsin_texcoords - r_offset - g_offset ).b;

    vec3 color = vec3(r, g, b);

    fsout_frag_color = vec4(color, 1.0);
}
