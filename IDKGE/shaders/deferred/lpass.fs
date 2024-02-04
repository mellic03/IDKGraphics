#version 460 core
#extension GL_GOOGLE_include_directive: require

#define DIRSHADOW_AMBIENT  1
#define MIPLEVEL_SPECULAR  5.0

layout (location = 0) out vec4 fsout_frag_color;



#include "../UBOs/UBOs.glsl"
#include "../vxgi/vxgi.glsl"
#include "../include/lightsource.glsl"


uniform sampler3D       un_voxel_radiance;
uniform sampler3D       un_voxel_albedo;
uniform sampler2DShadow un_vxgi_depthmap;
uniform mat4            un_vxgi_light_matrix;




in vec2 fsin_texcoords;

uniform sampler2D un_texture_0;
uniform sampler2D un_texture_1;
uniform sampler2D un_texture_2;
uniform sampler2D un_texture_3;

uniform samplerCube un_skybox_diffuse;
uniform samplerCube un_skybox_specular;
uniform sampler2D   un_BRDF_LUT;


#define PBR_ON 1

#define SKYBOX_IBL 0

#define VXGI_ON 1
#define VXGI_DIFFUSE 1
#define VXGI_DIFFUSE_APERTURE 60.0
#define VXGI_DIFFUSE_OFFSET 0.15

#define VXGI_SPECULAR 1
#define VXGI_SPECULAR_APERTURE 120.0
#define VXGI_SPECULAR_OFFSET 0.15

#define VXGI_AMBIENTAO 1
#define VXGI_AMBIENTAO_ONLY 0
#define VXGI_OFFSET 0.15



vec3 pointlight_contribution( int idx, vec3 position, vec3 F0, vec3 N, vec3 V, vec3 R,
                              vec3 albedo, float metallic, float roughness, float ao, float emission )
{
    PointLight light = ubo_pointlights[idx];
    vec3  light_position = light.position.xyz;
    vec3  light_ambient  = light.ambient.xyz;
    vec3  light_diffuse  = light.diffuse.xyz;
    vec3  falloff        = light.attenuation.xyz;

    const vec3 L  = normalize(light_position - position);
    const vec3 H  = normalize(V + L);

    float d = distance(light_position, position);
    float attenuation = 1.0 / (falloff.x + d*falloff.y + d*d*falloff.z);
    vec3  radiance    = light_diffuse * attenuation;

    float ndf = NDF(roughness, N, H);
    float G = GSF(roughness, N, V, L);
    vec3  F = fresnelSchlickR(max(dot(H, V), 0.0), F0, roughness);

    vec3  numerator   = ndf * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + EPSILON;
    vec3  specular    = numerator / denominator;

    vec3 Ks = F;
    vec3 Kd = (vec3(1.0) - Ks) * (1.0 - metallic);

    float NdotL = max(dot(N, L), 0.0);

    return emission*albedo + (Kd * albedo / PI + specular) * radiance * NdotL;
}


vec3 orthogonal( vec3 u )
{
	u = normalize(u);
	vec3 v = vec3(0.99146, 0.11664, 0.05832);
	return abs(dot(u, v)) > 0.99999 ? cross(u, vec3(0, 1, 0)) : cross(u, v);
}


vec3 indirect_diffuse( vec3 origin, vec3 N )
{
    float aperture     = radians(VXGI_DIFFUSE_APERTURE);
    vec3  VXGI_diffuse = vec3(0.0);

    float alpha = 0.5;

    vec3 T  = normalize(orthogonal(N));
    vec3 B  = normalize(cross(N, T));
    vec3 Tp = normalize(0.5 * (T + B));
    vec3 Bp = normalize(0.5 * (T - B));


    vec3 cone_directions[9] = vec3[](
        N,
        normalize(mix(N,  T, alpha)),
        normalize(mix(N, -T, alpha)),
        normalize(mix(N,  B, alpha)),
        normalize(mix(N, -B, alpha)),

        normalize(mix(N,  Tp, alpha)),
        normalize(mix(N, -Tp, alpha)),
        normalize(mix(N,  Bp, alpha)),
        normalize(mix(N, -Bp, alpha))
    );

    for (int i=0; i<9; i++)
    {
        vec3 cone_dir = cone_directions[i];

        VXGI_diffuse += VXGI_TraceCone(
            origin,
            cone_dir,
            aperture,
            un_viewpos,
            un_voxel_radiance
        );
    }

    return VXGI_diffuse;
}


vec3 trace_specular( vec3 origin, vec3 N, vec3 R, float roughness )
{
    float aperture = clamp(roughness * radians(VXGI_SPECULAR_APERTURE), 0.01, 6.0);
    return VXGI_TraceCone(origin, R, aperture, un_viewpos, un_voxel_radiance);
}


float trace_ao( vec3 origin, vec3 N )
{
    float aperture = radians(VXGI_DIFFUSE_APERTURE);
    float VXGI_AO  = 0.0;

    float CONE_ANGLE = 0.5;
    vec3 T = normalize(orthogonal(N));
    vec3 B = normalize(cross(N, T));

    vec3 cone_directions[5] = vec3[](
        N,
        normalize(mix(N,  T, CONE_ANGLE)),
        normalize(mix(N, -T, CONE_ANGLE)),
        normalize(mix(N,  B, CONE_ANGLE)),
        normalize(mix(N, -B, CONE_ANGLE))
    );

    for (int i=0; i<1; i++)
    {
        vec3 cone_dir = cone_directions[i];
        VXGI_AO += VXGI_TraceAO(origin + VXGI_OFFSET*N, cone_dir, aperture, un_viewpos, un_voxel_albedo);
    }

    return 1.0 - clamp(VXGI_AO/1.0, 0.0, 1.0);
}


void main()
{
    vec3 result = vec3(0.0);


    vec4  albedo_a = texture(un_texture_0, fsin_texcoords);
    vec3  albedo   = albedo_a.rgb;
    float alpha    = albedo_a.a;

    vec3  position = texture(un_texture_1, fsin_texcoords).xyz;
    vec3  normal   = texture(un_texture_2, fsin_texcoords).xyz;

    vec4  texture_pbr = texture( un_texture_3, fsin_texcoords);
    float roughness   = clamp(texture_pbr.r, 0.01, 0.99);
    float metallic    = clamp(texture_pbr.g, 0.01, 0.99);
    float ao          = clamp(texture_pbr.b, 0.01, 0.99);
    float emission    = clamp(texture_pbr.a, 0.01, 0.99);

    if (alpha < 1.0)
    {
       fsout_frag_color = vec4(0.0);
       return;
    }

    vec3  N     = normalize(normal);
    vec3  V     = normalize(un_viewpos - position);
    vec3  R     = reflect(-V, N); 
    vec3  F0    = mix(vec3(0.04), albedo, metallic);
    float NdotV = max(dot(N, V), 0.0);
    vec3  F     = fresnelSchlick(NdotV, F0);
    vec3  Ks    = F;
    vec3  Kd    = (vec3(1.0) - Ks) * (1.0 - metallic);
    vec2  brdf  = texture(un_BRDF_LUT, vec2(NdotV, roughness)).rg;


    #if PBR_ON == 0
        result += dirlight_contribution_no_pbr(0, position, N, V, R, albedo);
        result *= dirlight_shadow(0, un_view, position, N);
        fsout_frag_color = vec4(result, 1.0);
        return;
    #endif


    vec3  dir_lighting = dirlight_contribution(0, position, F0, N, V, R, albedo, metallic, roughness);
    float dir_shadow = dirlight_shadow(0, un_view, position, N);
    // float dir_shadow = dirlight_shadow_2(0, un_vxgi_depthmap, un_view, un_vxgi_light_matrix, position, N);

    result += dir_lighting * dir_shadow;


    // IBL
    // -----------------------------------------------------------------------------------------
    #if SKYBOX_IBL == 1
        vec3 irradiance = textureLod(un_skybox_diffuse, N, roughness).rgb;
        vec3 diffuse    = irradiance * albedo;

        vec3 prefilter = textureLod(un_skybox_specular, R, roughness*MIPLEVEL_SPECULAR).rgb;
        vec3 specular  = prefilter * (Ks * brdf.x + brdf.y);
        vec3 ambient   = (Kd * diffuse + specular) * ao;

        #if DIRSHADOW_AMBIENT == 1
            ambient *= clamp(dir_shadow, un_dirlights[0].ambient.w, 1.0);
        #endif

        result += ambient;
    #endif
    // -----------------------------------------------------------------------------------------


    // Cone Tracing
    // -----------------------------------------------------------------------------------------
    #if VXGI_ON == 1
        if (VXGI_in_bounds(position, un_viewpos))
        {
            vec3  VXGI_ambient  = vec3(0.0);
            vec3  VXGI_diffuse  = vec3(0.0);
            vec3  VXGI_specular = vec3(0.0);
            float VXGI_AO       = 0.0;

            #if VXGI_DIFFUSE == 1
                VXGI_diffuse = indirect_diffuse(position, N);
                VXGI_diffuse = VXGI_diffuse * albedo;
            #endif

            #if VXGI_SPECULAR == 1
                VXGI_specular = trace_specular(position, N, R, roughness);
                VXGI_specular = VXGI_specular * (Ks * brdf.x + brdf.y);
            #endif

            VXGI_ambient = (Kd * VXGI_diffuse.rgb + VXGI_specular);
            VXGI_AO      = trace_ao(position, N);

            result += VXGI_ambient;

            #if VXGI_AMBIENTAO == 1
                result *= (VXGI_AO);
            #endif

            #if VXGI_AMBIENTAO_ONLY == 1
                result = vec3(VXGI_AO);
            #endif

        }
    #endif
    // -----------------------------------------------------------------------------------------

    // result += albedo * 2.0 * clamp(emission - 1.0, 0.0, 1.0);


    // fsout_frag_color = vec4(vec3(roughness), 1.0);
    fsout_frag_color = vec4(result, 1.0);

}
