#version 460 core
#extension GL_ARB_bindless_texture: require
#extension GL_GOOGLE_include_directive: require

layout (location = 0) out vec4 fsout_frag_color;


#include "../include/SSBO_indirect.glsl"
#include "../include/UBOs.glsl"
#include "../include/util.glsl"
#include "../include/pbr.glsl"

#define MIPLEVEL_SPECULAR 4.0
#define SKYBOX_IBL_STRENGTH 1.0

in vec3 fsin_fragpos;
flat in int idk_LightID;


uniform sampler2D un_texture_0;
uniform sampler2D un_texture_1;
uniform sampler2D un_texture_2;

uniform sampler2D un_fragdepth;
uniform sampler2D un_BRDF_LUT;

uniform samplerCube un_skybox_diffuse;
uniform samplerCube un_skybox_specular;

uniform sampler2D un_shadowmap;




// float sampleDepthMap_2( vec3 uv, float bias )
// {
//     const int KERNEL_HW = 3;

//     vec2 texelSize = 0.5 / textureSize(un_shadowmap, 0).xy;

//     float shadow = 0.0;

//     for(int x = -KERNEL_HW; x <= KERNEL_HW; ++x)
//     {
//         for(int y = -KERNEL_HW; y <= KERNEL_HW; ++y)
//         {
//             vec2 sample_uv    = uv.xy + vec2(x, y) * texelSize;
//             // vec2 sample_uv    = uv.xy;
//             vec3 sample_coord = vec3(sample_uv, uv.z - bias);

//             float depth = texture(un_shadowmap, sample_coord.xy).r;

//             shadow += (depth - bias > )
//         }
//     }

//     // return shadow;
//     return shadow / ((2*KERNEL_HW+1)*(2*KERNEL_HW+1));
// }



float dirlight_shadow_2( IDK_Camera camera, IDK_Dirlight light, mat4 view_matrix, vec3 position, vec3 N )
{
    vec3 L = normalize(-light.direction.xyz);

    vec3  fragpos_viewspace  = (view_matrix * vec4(position, 1.0)).xyz;
    vec4  fragpos_lightspace = light.transform * vec4(position, 1.0);

    vec3  projCoords = fragpos_lightspace.xyz / fragpos_lightspace.w;
          projCoords = projCoords * 0.5 + 0.5;

    float frag_depth = projCoords.z;
    float bias   = 0.01; // * max(dot(N, L), 0.001);


    const int KERNEL_HW = 1;
    vec2 texelSize = 1.0 / textureSize(un_shadowmap, 0).xy;
    float shadow = 0.0;

    for(int x = -KERNEL_HW; x <= KERNEL_HW; ++x)
    {
        for(int y = -KERNEL_HW; y <= KERNEL_HW; ++y)
        {
            vec2 sample_uv = projCoords.xy + vec2(x, y) * texelSize;
            float depth = texture(un_shadowmap, sample_uv).r;

            shadow += (frag_depth - bias > depth) ? 1.0 : 0.0;
        }
    }

    return 1.0 - shadow / ((2*KERNEL_HW+1)*(2*KERNEL_HW+1));
}



float ScreenspaceShadow( IDK_Camera camera, IDK_PBRSurfaceData surface, IDK_Dirlight light )
{
    vec3 ray_pos = surface.position.xyz;
    vec3 ray_dir = normalize(-light.direction.xyz);
    mat4 PV = camera.PV;

    float occlusion = 0.0;

    const float RAY_RANGE = 0.25;
    const int   RAY_STEPS = 32;
    const float RAY_STEP  = (RAY_RANGE / RAY_STEPS);

    vec2 uv;


    for (int i=0; i<RAY_STEPS; i++)
    {
        ray_pos += RAY_STEP*ray_dir;

        // Project ray into UV space
        // ---------------------------------------------------------------------------
        vec4 projected = PV * vec4(ray_pos, 1.0);
        projected.xy /= projected.w;
        projected.xy = projected.xy * 0.5 + 0.5;

        ivec2 tx = ivec2(projected.xy * camera.image_size.xy);
              uv = projected.xy;


        // float frag_depth = (PV * imageLoad(un_position, tx)).z;
        vec3 pos = IDK_WorldFromDepth(un_fragdepth, uv, camera.P, camera.V);

        float frag_depth = (PV * vec4(pos, 1.0)).z;
        float ray_depth  = IDK_WorldToUV(ray_pos, PV).z;
        // ---------------------------------------------------------------------------

        if (ray_depth >= frag_depth && (ray_depth-frag_depth < 0.01))
        {
            occlusion = 1.0;
            break;
        }
    }

    float dist = distance(uv, vec2(0.5));
          dist = (dist / camera.image_size.x);

    return (0.5 - dist) * (1.0 - occlusion);
}



void main()
{
    IDK_Camera   camera = IDK_RenderData_GetCamera();
    IDK_Dirlight light  = IDK_RenderData_GetDirlight(idk_LightID);

    vec2 texcoord = IDK_WorldToUV(fsin_fragpos, camera.P * camera.V).xy;
    vec3 worldpos = IDK_WorldFromDepth(un_fragdepth, texcoord, camera.P, camera.V);
    // float depth = texture(un_fragdepth, texcoord).r;

    IDK_PBRSurfaceData surface = IDK_PBRSurfaceData_load(
        camera,
        texcoord,
        un_fragdepth,
        un_texture_0, 
        un_texture_1,
        un_texture_2,
        un_BRDF_LUT
    );


    float shadow  = dirlight_shadow_2(camera, light, camera.V, worldpos, surface.N);
    // float sss     = ScreenspaceShadow(camera, surface, light);
    //       shadow  = min(shadow, sss);

    vec3  result  = shadow * IDK_PBR_Dirlight2(light, surface, worldpos);
          result += light.ambient.rgb * surface.albedo.rgb;


    // IBL
    // -----------------------------------------------------------------------------------------
    vec3 IBL_contribution = vec3(0.0);

    vec3 irradiance = textureLod(un_skybox_diffuse, surface.N, surface.roughness).rgb;
    vec3 diffuse    = irradiance * surface.albedo;

    vec3 prefilter = textureLod(un_skybox_specular, surface.R, surface.roughness*MIPLEVEL_SPECULAR).rgb;
    vec3 specular  = prefilter * surface.brdf;
    vec3 ambient   = (surface.Kd * diffuse + specular) * surface.ao;

    // #if DIRSHADOW_AMBIENT == 1
        ambient *= clamp(shadow, light.ambient.w, 1.0);
    // #endif

    IBL_contribution = SKYBOX_IBL_STRENGTH * ambient;
    result += IBL_contribution;
    // -----------------------------------------------------------------------------------------

    result += (surface.emission * surface.albedo);

    fsout_frag_color = vec4(result, surface.alpha);
}
