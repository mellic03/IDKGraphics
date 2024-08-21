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

uniform samplerCube un_skybox;
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



float IDK_PHG( float g, float cosTheta )
{
    const float Inv4Pi = 0.07957747154594766788;
    
    float gSq = g * g;
    float denomPreMul = 1 + gSq - (2.0 * g * cosTheta);
    return (1 - gSq) * Inv4Pi * inversesqrt(denomPreMul * denomPreMul * denomPreMul);
}

float IDK_MieScattering( float cosTheta )
{
    return mix(IDK_PHG(0.8, cosTheta), IDK_PHG(-0.5, cosTheta), 0.5);
}

float IDK_RayleighScattering( float cosTheta )
{
    return ((3.0*3.14159) / 16.0) * (1.0 + cosTheta*cosTheta);
}



#define V_color  vec3(0.05, 0.13, 0.22)
#define H_color1 vec3(0.59, 0.4, 0.17)
#define H_color2 vec3(0.7, 0.76, 0.66)


vec3 bad_atmospherics( vec3 ray_dir, vec3 L, IDK_Dirlight light )
{
    vec3 result = vec3(0.0);

    float VdotL    = dot(ray_dir, -L) * 0.5 + 0.5;
    float vertical = pow(1.0 - abs(ray_dir.y), 5);

    vec3 hoz = mix(H_color1, H_color2, pow(VdotL, 2));
    result = mix(V_color, hoz, vertical);

    VdotL = clamp(dot(ray_dir, -L), 0.0, 1.0);
    result += light.diffuse.a * IDK_MieScattering(VdotL);

    return result;
}



void main()
{
    IDK_Camera   camera = IDK_UBO_cameras[0];
    IDK_Dirlight light  = IDK_UBO_dirlights[lightID];

    vec2  texcoord = IDK_WorldToUV(fsin_fragpos, camera.P * camera.V).xy;

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
    vec3  ray_dir = normalize(worldpos - camera.position.xyz);

    float shadow = dirlight_shadow_2(camera, light, camera.V, worldpos, surface.N);
    vec3  result  = IDK_PBR_Dirlight(light, surface, worldpos);
          result *= shadow;
          result += light.ambient.rgb * surface.albedo.rgb;


    // IBL
    // -----------------------------------------------------------------------------------------
    vec3 IBL_contribution = vec3(0.0);

    vec3 irradiance = textureLod(un_skybox_diffuse, surface.N, surface.roughness).rgb;
    vec3 diffuse    = irradiance * surface.albedo;

    vec3 prefilter = textureLod(un_skybox_specular, surface.R, surface.roughness*4.0).rgb;
    vec3 specular  = prefilter * surface.brdf;
    vec3 ambient   = (surface.Kd * diffuse + specular) * surface.ao;

    // #if DIRSHADOW_AMBIENT == 1
        // ambient *= clamp(shadow, light.ambient.w, 1.0);
    // #endif

    result += ambient * light.ambient.w;
    // -----------------------------------------------------------------------------------------


    if (distance(camera.position.xyz, worldpos) >= 0.95 * camera.far)
    {
        vec3 result = textureLod(un_skybox, ray_dir, 0.0).rgb; // bad_atmospherics(ray_dir, light.direction.xyz, light);
        fsout_frag_color = vec4(result, 1.0);
        return;
    }

    // result += surface.emission * surface.albedo;

    fsout_frag_color = vec4(result, 1.0);
}





