#version 460 core

#extension GL_GOOGLE_include_directive: require
#include "../include/storage.glsl"
#include "../include/util.glsl"
#include "../include/pbr.glsl"

layout (location = 0) out vec4 fsout_frag_color;

in vec3 fsin_fragpos;
flat in uint lightID;

uniform sampler2D un_texture_0;
uniform sampler2D un_texture_1;
uniform sampler2D un_texture_2;

uniform sampler2D un_fragdepth;
uniform sampler2D un_BRDF_LUT;

uniform samplerCube un_skybox_diffuse;
uniform samplerCube un_skybox_specular;

uniform sampler2DArray un_shadowmap;



float dirlight_shadow_2( IDK_Camera camera, IDK_Dirlight light, mat4 V, vec3 position, vec3 N )
{
    vec3 L = normalize(-light.direction.xyz);

    vec3 fragpos_viewspace = (V * vec4(position, 1.0)).xyz;
    vec4 cascade_depths = light.cascades;
    vec4 res     = step(cascade_depths, vec4(abs(fragpos_viewspace.z)));
    int  cascade = int(res.x + res.y + res.z + res.w);
         cascade = clamp(cascade, 0, 3);

    vec4  fragpos_lightspace = light.transforms[cascade] * vec4(position, 1.0);
    vec3  projCoords = fragpos_lightspace.xyz / fragpos_lightspace.w;
          projCoords = projCoords * 0.5 + 0.5;

    float frag_depth = abs(projCoords.z);
    float bias = 0.01 * max(dot(N, L), 0.001);
          bias *= 1 / (cascade_depths[cascade] * 0.5);

    const int KERNEL_HW = 4;
    vec2 texelSize = 1.0 / textureSize(un_shadowmap, 0).xy;
    float shadow = 0.0;

    for(int x = -KERNEL_HW; x <= KERNEL_HW; x++)
    {
        for(int y = -KERNEL_HW; y <= KERNEL_HW; y++)
        {
            vec2  sample_uv    = texelSize * vec2(x, y) + projCoords.xy;
            float sample_depth = texture(un_shadowmap, vec3(sample_uv, cascade)).r;

            shadow += (frag_depth - bias) > sample_depth ? 1.0 : 0.0;        
        }    
    }
    shadow /= ((KERNEL_HW*2+1)*(KERNEL_HW*2+1));

    return 1.0 - shadow;
}




void main()
{
    IDK_Camera   camera = IDK_UBO_cameras[0];
    IDK_Dirlight light  = IDK_UBO_dirlights[lightID];

    vec2 texcoord = IDK_WorldToUV(fsin_fragpos, camera.P * camera.V).xy;

    IDK_PBRSurfaceData surface = IDK_PBRSurfaceData_load(
        camera,
        texcoord,
        un_fragdepth,
        un_texture_0, 
        un_texture_1,
        un_texture_2,
        un_BRDF_LUT
    );

    vec3 worldpos = surface.position;

    float shadow = dirlight_shadow_2(camera, light, camera.V, worldpos, surface.N);
    vec3  result  = IDK_PBR_Dirlight(light, surface, worldpos);
          result *= shadow;
          result += light.ambient.rgb * surface.albedo.rgb;


    // // IBL
    // // -----------------------------------------------------------------------------------------
    // vec3 IBL_contribution = vec3(0.0);

    // vec3 irradiance = textureLod(un_skybox_diffuse, surface.N, surface.roughness).rgb;
    // vec3 diffuse    = irradiance * surface.albedo;

    // vec3 prefilter = textureLod(un_skybox_specular, surface.R, surface.roughness*4.0).rgb;
    // vec3 specular  = prefilter * surface.brdf;
    // vec3 ambient   = (surface.Kd * diffuse + specular) * surface.ao;

    // // #if DIRSHADOW_AMBIENT == 1
    //     ambient *= clamp(shadow, light.ambient.w, 1.0);
    // // #endif

    // result += shadow * ambient;
    // // -----------------------------------------------------------------------------------------

    result += 2.0*surface.emission * surface.albedo;


    fsout_frag_color = vec4(result, surface.alpha);
}





