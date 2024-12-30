#version 460 core

#extension GL_GOOGLE_include_directive: require
#include "../include/storage.glsl"
#include "../include/util.glsl"
#include "../include/pbr.glsl"
#include "../include/taa.glsl"
#include "../include/volumetric.glsl"

layout (location = 0) out vec4 fsout_frag_color;


in vec3 fsin_fragpos;
flat in uint lightID;

uniform sampler2D un_occlusion;
uniform sampler2D un_texture_0;
uniform sampler2D un_texture_1;
uniform sampler2D un_texture_2;
uniform sampler2D un_fragdepth;
uniform sampler2D un_BRDF_LUT;

uniform samplerCube un_skybox;
uniform samplerCube un_skybox_diffuse;
uniform samplerCube un_skybox_specular;

uniform sampler2DArrayShadow un_shadowmap;

uniform float un_height_offset  = 0.0;
uniform float un_height_falloff = 1.0;
uniform float un_intensity      = 1.0;




float IDK_ShadowDirlight( IDK_Dirlight light, vec3 frag_viewspace, vec3 frag_worldspace, vec3 N )
{
    vec3 L = normalize(-light.direction.xyz);

    vec4 cascade_depths = light.cascades;
    vec4 res     = step(cascade_depths, vec4(abs(frag_viewspace.z)));
    int  cascade = int(res.x + res.y + res.z + res.w);
         cascade = clamp(cascade, 0, 4);

    vec4  texcoord     = light.transforms[cascade] * vec4(frag_worldspace, 1.0);
          texcoord.xyz = (texcoord.xyz / texcoord.w) * 0.5 + 0.5;

    float frag_depth = abs(texcoord.z);
    float bias = max(0.1 * dot(N, L), 0.02);
          bias /= (cascade_depths[cascade]);

    const int HW = 1;
    vec2  texelSize = 1.0 / textureSize(un_shadowmap, 0).xy;

    vec2  sample_uv = texcoord.xy;
    float shadow = texture(un_shadowmap, vec4(sample_uv, cascade, frag_depth-bias)).r;
    // float shadow = (frag_depth - bias) > sample_depth ? 1.0 : 0.0;

    // float shadow = 0.0;

    // for(int x = -HW; x <= HW; x++)
    // {
    //     for(int y = -HW; y <= HW; y++)
    //     {
    //         vec2  sample_uv    = texelSize * vec2(x, y) + jitter + projCoords.xy;
    //         float sample_depth = texture(un_shadowmap, vec3(sample_uv, cascade)).r;

    //         shadow += (frag_depth - bias) > sample_depth ? 1.0 : 0.0;        
    //     }    
    // }
    // shadow /= ((HW*2+1)*(HW*2+1));

    return 1.0 - shadow;
}




float dirlight_shadow_2( IDK_Camera camera, IDK_Dirlight light, mat4 V, vec3 position, vec3 N )
{
    const float[4] biases = float[4]( 0.005, 0.01, 0.001, 0.02 );

    vec3 L = normalize(-light.direction.xyz);

    vec3 fragpos_viewspace = (V * vec4(position, 1.0)).xyz;
    vec4 cascade_depths = light.cascades;

    // int cascade = 3;

    // for (int i=3; i>=0; i--)
    // {
    //     if (abs(fragpos_viewspace.z) < light.cascades[i])
    //     {
    //         cascade = i;
    //     }
    // }

    vec4 res     = step(cascade_depths, vec4(abs(fragpos_viewspace.z)));
    int  cascade = int(res.x + res.y + res.z + res.w);
         cascade = clamp(cascade, 0, 3);

    vec4  fragpos_lightspace = light.transforms[cascade] * vec4(position, 1.0);
    vec3  projCoords = fragpos_lightspace.xyz / fragpos_lightspace.w;
          projCoords = projCoords * 0.5 + 0.5;

    float frag_depth = abs(projCoords.z);
    float bias = biases[cascade] * max(dot(N, L), 0.001);
        //   bias /= (cascade_depths[cascade]);

    vec2  tsize = textureSize(un_shadowmap, 0).xy;
    float shadow = 0.0;

    vec2 texel = (tsize * projCoords.xy) + IDK_GetIrrational();

    vec2[25] offsets = vec2[25](
        vec2(-2, -2), vec2(-1, -2), vec2(0, -2), vec2(+1, -2), vec2(+2, -2),
        vec2(-2, -1), vec2(-1, -1), vec2(0, -1), vec2(+1, -1), vec2(+2, -1),
        vec2(-2,  0), vec2(-1,  0), vec2(0,  0), vec2(+1,  0), vec2(+2,  0),
        vec2(-2, +1), vec2(-1, +1), vec2(0, +1), vec2(+1, +1), vec2(+2, +1),
        vec2(-2, +2), vec2(-1, +2), vec2(0, +2), vec2(+1, +2), vec2(+2, +2)
    );

    for (int i=0; i<25; i++)
    {
        vec2 uv = projCoords.xy + offsets[i]/tsize;
        shadow += texture(un_shadowmap, vec4(uv, cascade, frag_depth-bias)).r;
    }

    shadow /= 25.0;

    // const int HW = 5;

    // for (int y=-HW; y<=HW; y++)
    // {
    //     for (int x=-HW; x<=HW; x++)
    //     {
    //         vec2 uv = projCoords.xy + (vec2(x, y) + noise) / tsize;
    //         shadow += texture(un_shadowmap, vec4(uv, cascade, frag_depth-bias)).r;
    //     }    
    // }

    // shadow /= ((HW*2+1)*(HW*2+1));

    return shadow;
}



// float get_ssetness( vec3 L )
// {
//     float ssetness = 1.0 - max(L.y, 0.0);
//           ssetness = pow(ssetness, 3.0);
//     return ssetness;
// }



// vec3 computeSomeShit( vec3 diffuse, vec2 texcoord, vec3 L, vec3 rp, vec3 rd )
// {
//     vec3 result = vec3(0.0);

//     rp -= vec3(0.0, un_height_offset, 0.0);


//     vec3  color = VolumetricFogColor(L, rd);

//     if (textureLod(un_fragdepth, texcoord, 0.0).r >= 1.0)
//     {
//         vec3 sunset = pow(color, vec3(2.0));
    
//         float sunsetness = 0.15 * max(dot(rd, vec3(0, 1, 0)), 0.0);
//         vec3  blueness   = pow(vec3(0.4, 0.8, 1.0)+sunsetness, vec3(4.0));

//         color += blueness;


//         float sunness = max(dot(L, rd), 0.0);
//               sunness = pow(sunness, 256.0);
            
//         vec3 sun = vec3(1.0, 0.95, 0.85);
//              sun = pow(sun, vec3(4.0));
            
//         result = mix(result, diffuse*sun, sunness);
//     }

//     vec3 ree = vec3(255, 70, 0) / 255.0;

//     float ssetness = get_ssetness(L);
//     ree = diffuse * mix(vec3(1.0), ree, ssetness);

//     return ree*color;
// }


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

    surface.ao *= textureLod(un_occlusion, texcoord, 0.0).r;

    vec3 worldpos = surface.position;

    float shadow  = dirlight_shadow_2(camera, light, camera.V, worldpos, surface.N);
    vec3  result  = IDK_PBR_Dirlight(light, surface, worldpos);
          result *= shadow;
          result += light.ambient.rgb * surface.albedo.rgb * surface.ao;


    // IBL
    // -----------------------------------------------------------------------------------------
    vec3 IBL_contribution = vec3(0.0);

    vec3 irradiance = textureLod(un_skybox_diffuse, surface.N, surface.roughness).rgb;
    vec3 diffuse    = surface.Kd * irradiance * surface.albedo;
    vec3 prefilter  = textureLod(un_skybox_specular, surface.R, surface.roughness*4.0).rgb;
    vec3 specular   = prefilter * surface.brdf;
    vec3 ambient    = (diffuse + specular) * surface.ao;

    // #if DIRSHADOW_AMBIENT == 1
        ambient *= clamp(shadow, light.ambient.w, 1.0);
    // #endif

    result += ambient * light.ambient.w;
    // -----------------------------------------------------------------------------------------

    result += surface.emission * surface.albedo;

    if (texture(un_fragdepth, texcoord).r >= 1.0)
    {
        result = textureLod(un_skybox, -surface.V, 0.0).rgb;
    }

    fsout_frag_color = vec4(result, 1.0);
}





