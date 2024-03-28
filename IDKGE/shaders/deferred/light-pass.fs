#version 460 core
#extension GL_ARB_bindless_texture: require
#extension GL_GOOGLE_include_directive: require

#define DIRSHADOW_AMBIENT  0
#define MIPLEVEL_SPECULAR  4.0

layout (location = 0) out vec4 fsout_frag_color;
// layout (binding=30, rg8i) writeonly uniform iimage2D un_velocity;


#include "../include/UBOs.glsl"

#include "../include/util.glsl"
#include "../include/pbr.glsl"
#include "../include/lightsource.glsl"

#include "../include/SSBO_indirect.glsl"



in vec2 fsin_texcoords;

uniform sampler2D un_texture_0;
uniform sampler2D un_texture_1;
uniform sampler2D un_texture_2;
uniform sampler2D un_fragdepth;
uniform sampler2D un_BRDF_LUT;

uniform samplerCube un_skybox_diffuse;
uniform samplerCube un_skybox_specular;


#define PBR_ON 1

#define SKYBOX_IBL 0
#define SKYBOX_IBL_STRENGTH 1



void main()
{
    IDK_Camera camera = IDK_RenderData_GetCamera();

    IDK_PBRSurfaceData surface = IDK_PBRSurfaceData_load(
        camera,
        fsin_texcoords,
        un_fragdepth,
        un_texture_0,
        un_texture_1,
        un_texture_2,
        un_BRDF_LUT
    );


    if (surface.alpha < 1.0)
    {
       discard;
    }


    // IDK_Dirlight dirlight =  {
    //     un_dirlights[0].direction,
    //     un_dirlights[0].ambient,
    //     un_dirlights[0].diffuse
    // };


    vec3 result = vec3(0.0);

    // vec3  dir_lighting = IDK_PBR_Dirlight(dirlight, surface, surface.position);
    // float dir_shadow   = dirlight_shadow(0, camera.V, surface.position, surface.N);

    // result += dir_lighting * dir_shadow;
    // result += surface.albedo * un_dirlights[0].ambient.rgb;


    // IBL
    // -----------------------------------------------------------------------------------------
    // vec3 IBL_contribution = vec3(0.0);

    // #if SKYBOX_IBL == 0
    //     vec3 irradiance = textureLod(un_skybox_diffuse, N, roughness).rgb;
    //     vec3 diffuse    = irradiance * albedo;

    //     vec3 prefilter = textureLod(un_skybox_specular, R, roughness*MIPLEVEL_SPECULAR).rgb;
    //     vec3 specular  = prefilter * brdf;
    //     vec3 ambient   = (Kd * diffuse + specular) * ao;

    //     // #if DIRSHADOW_AMBIENT == 1
    //     //     ambient *= clamp(dir_shadow, un_dirlights[0].ambient.w, 1.0);
    //     // #endif

    //     IBL_contribution = SKYBOX_IBL_STRENGTH * ambient;
    // #endif

    // result += IBL_contribution;
    // -----------------------------------------------------------------------------------------


    // // Cone Tracing
    // // -----------------------------------------------------------------------------------------
    // #if VXGI_ON == 1
    //     if (VXGI_in_bounds(position, un_RenderData.cameras[0].position.xyz))
    //     {
    //         vec3  VXGI_ambient  = vec3(0.0);
    //         vec3  VXGI_diffuse  = vec3(0.0);
    //         vec3  VXGI_specular = vec3(0.0);
    //         float VXGI_AO       = 0.0;

    //         #if VXGI_DIFFUSE == 1
    //             VXGI_diffuse = VXGI_IndirectDiffuse(
    //                 position,
    //                 N,
    //                 0.0,
    //                 un_RenderData.cameras[0].position.xyz,
    //                 un_voxel_radiance
    //             );

    //             VXGI_diffuse = VXGI_diffuse * albedo;
    //         #endif

    //         #if VXGI_SPECULAR == 1
    //             VXGI_specular = trace_specular(position, N, R, roughness);
    //             VXGI_specular = VXGI_specular * (Ks * brdf.x + brdf.y);
    //         #endif

    //         VXGI_ambient = (Kd * VXGI_diffuse.rgb + VXGI_specular);

    //         result += VXGI_ambient;
    //     }
    // #endif
    // // -----------------------------------------------------------------------------------------

    result += 10.0 * surface.emission * surface.albedo;

    result = (result == result) ? result : vec3(0.0);

    fsout_frag_color = vec4(result, 1.0);
}
