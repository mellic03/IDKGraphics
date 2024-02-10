#version 460 core

#extension GL_ARB_bindless_texture: require
layout (std430, binding = 8) buffer SBO_bindless_textures
{
    sampler2D un_bindless_samplers[128];
};
uniform int un_material_id;


layout (location = 0) out vec4 fsout_albedo;
layout (location = 1) out vec4 fsout_position;
layout (location = 2) out vec4 fsout_normal;
layout (location = 3) out vec4 fsout_pbr;

in vec3 fsin_fragpos;
in vec3 fsin_normal;
in vec3 fsin_tangent;
in vec2 fsin_texcoords;


struct Camera
{
    vec4 position;
    vec4 beg;
    vec4 aberration_rg;
    vec4 aberration_b;
    vec4 exposure;
};

layout (std140, binding = 2) uniform UBO_camera_data
{
    mat4 un_view;
    mat4 un_projection;
    vec3 un_viewpos;
    Camera un_camera;
};


in vec3 TBN_viewpos;
in vec3 TBN_fragpos;
in mat3 TBN;
in mat3 TBNT;


#define PARALLAX 0
#define parallax_steps 8.0


struct vec5
{
    vec2 uv;
    vec3 fragpos;
};


struct DirLight
{
    vec4 direction;
    vec4 ambient;
    vec4 diffuse;
};


layout (std140, binding = 5) uniform UBO_dirlights
{
    DirLight    ubo_dirlights[10];
    mat4        un_cascade_matrices[4];
};
uniform sampler2D   un_dirlight_depthmaps[10];



// vec2 parallax_map()
// {
//     const float strength = un_material.displacement_strength;

//     vec3 viewdir = normalize(TBNT*un_viewpos - TBNT*fsin_fragpos);
//     viewdir.y *= -1.0;

//     vec2 p = strength * (viewdir.xy / viewdir.z);

//     const float layer_height = 1.0 / parallax_steps;
//     vec2 delta = p / parallax_steps;

//     vec2  prev_uv = fsin_texcoords;
//     vec2  uv = fsin_texcoords;
//     float texture_depth = texture(un_material.displacement, uv).r;
//     float prev_texture_depth = texture_depth;
//     float layer_depth   = 0.0;

//     while (layer_depth < texture_depth)
//     {
//         prev_uv = uv;
//         uv += delta;

//         prev_texture_depth = texture_depth;
//         texture_depth = texture(un_material.displacement, uv).r;

//         layer_depth += layer_height;
//     }

//     float afterDepth  = texture_depth - layer_depth;
//     float beforeDepth = prev_texture_depth - layer_depth + layer_height;
//     float a = afterDepth / (afterDepth - beforeDepth);

//     vec2 final_uv = a*prev_uv + (1.0 - a)*uv;

//     return final_uv;
// }



void main()
{
    // #if PARALLAX == 1
    //     vec2 texcoords = parallax_map();
    // #else
        vec2 texcoords = fsin_texcoords;
    // #endif

    int albedo_idx = 4*un_material_id + 0;
    int normal_idx = 4*un_material_id + 1;
    int ao_r_m_idx = 4*un_material_id + 2;
    int emission_idx = 4*un_material_id + 3;

    vec4  albedo   = texture(un_bindless_samplers[albedo_idx], texcoords).rgba;
    vec3  ao_r_m   = texture(un_bindless_samplers[ao_r_m_idx], texcoords).rgb;
    float emission = texture(un_bindless_samplers[emission_idx], texcoords).r;
    vec3  normal   = texture(un_bindless_samplers[normal_idx], texcoords).xyz * 2.0 - 1.0;

    if (albedo.a < 1.0)
        discard;

    float ao        = ao_r_m.r;
    float roughness = ao_r_m.g;
    float metallic  = ao_r_m.b;

    float a = 0.25; // un_material.normal_strength;
    vec3 N = normalize(TBN * normal);
         N = normalize(a*N + (1.0 - a)*normalize(fsin_normal));
        //  N = normalize(fsin_normal);

    fsout_albedo   = vec4(albedo.rgb, 1.0);
    fsout_position = vec4(fsin_fragpos, 1.0);
    fsout_normal   = vec4(N, 1.0);
    fsout_pbr      = vec4(roughness, metallic, ao, emission.x);
}
