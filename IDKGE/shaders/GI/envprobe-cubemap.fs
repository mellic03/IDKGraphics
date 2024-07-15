#version 460 core

#extension GL_GOOGLE_include_directive: require
#include "../include/storage.glsl"
#include "../include/util.glsl"
#include "../include/pbr.glsl"

layout (location = 0) out vec4 fsout_radiance;

in vec3 fsin_fragpos;
in vec3 fsin_normal;
in vec2 fsin_texcoords;

flat in uint drawID;



uniform sampler2DArray un_shadowmap;
uniform sampler2D un_BRDF_LUT;

uniform samplerCube un_skybox_diffuse;
uniform samplerCube un_skybox_specular;


uniform mat4 un_projection;
uniform mat4 un_view;
uniform vec3 un_viewpos;



float dirlight_shadow_2( IDK_Dirlight light, mat4 view_matrix, vec3 position, vec3 N )
{
    vec3 L = normalize(-light.direction.xyz);

    vec3 fragpos_viewspace = (view_matrix * vec4(position, 1.0)).xyz;
    int  cascade = 4;

    vec4  fragpos_lightspace = light.transform * vec4(position, 1.0);

    vec3  projCoords = fragpos_lightspace.xyz / fragpos_lightspace.w;
          projCoords = projCoords * 0.5 + 0.5;

    float frag_depth = projCoords.z;
    float bias   = 0.0001; // * max(dot(N, L), 0.001);

    const int KERNEL_HW = 1;
    vec2 texelSize = 1.0 / textureSize(un_shadowmap, 0).xy;
    float shadow = 0.0;

    for(int x = -KERNEL_HW; x <= KERNEL_HW; ++x)
    {
        for(int y = -KERNEL_HW; y <= KERNEL_HW; ++y)
        {
            vec2 sample_uv = projCoords.xy + vec2(x, y) * texelSize;
            float depth = texture(un_shadowmap, vec3(sample_uv, cascade)).r;

            shadow += (frag_depth - bias > depth) ? 1.0 : 0.0;
        }
    }

    return 1.0 - shadow / ((2*KERNEL_HW+1)*(2*KERNEL_HW+1));
}



IDK_PBRSurfaceData
IDK_PBRSurfaceData_load_view( mat4 proj, mat4 view, vec3 viewpos, vec3 worldpos,
                              vec3 albedo, float alpha, vec3 normal,
                              float roughness, float metallic, float ao, float emission,
                              sampler2D BRDF_LUT )
{
    IDK_PBRSurfaceData data;

    vec3  N     = normalize(normal);
    vec3  V     = normalize(viewpos - worldpos);
    vec3  R     = reflect(-V, N); 
    vec3  F0    = clamp(mix(vec3(0.04), albedo, metallic), 0.0, 1.0);
    float NdotV = PBR_DOT(N, V);
    vec3  F     = fresnelSchlickR(NdotV, F0, roughness);
    vec3  Ks    = F;
    vec3  Kd    = (vec3(1.0) - Ks) * (1.0 - metallic);
    vec2  BRDF  = texture(BRDF_LUT, vec2(NdotV, roughness)).rg;
    vec3  brdf  = (Ks * BRDF.x + BRDF.y);


    data.position   = worldpos;

    data.albedo     = albedo;
    data.alpha      = alpha;

    data.roughness  = roughness;
    data.metallic   = metallic;
    data.ao         = ao;
    data.emission   = emission;

    data.N      = N;
    data.V      = V;
    data.R      = R;
    data.F0     = F0;
    data.NdotV  = NdotV;
    data.F      = F;
    data.Ks     = Ks;
    data.Kd     = Kd;
    data.brdf   = brdf;

    return data;
}




void main()
{
    vec2 texcoord = fsin_texcoords;
    uint offset   = IDK_SSBO_texture_offsets[drawID];

    vec4  albedo = texture(IDK_SSBO_textures[offset+0], texcoord).rgba;
    vec3  ao_r_m = texture(IDK_SSBO_textures[offset+2], texcoord).rgb;
    float noidea = texture(IDK_SSBO_textures[offset+3], texcoord).r;
    float emissv = texture(IDK_SSBO_textures[offset+4], texcoord).r;
    float ao        = ao_r_m.r;
    float roughness = ao_r_m.g;
    float metallic  = ao_r_m.b;

    IDK_Dirlight light = IDK_UBO_dirlights[0];

    IDK_PBRSurfaceData surface = IDK_PBRSurfaceData_load_view(
        un_projection, un_view, un_viewpos,
        fsin_fragpos,
        albedo.rgb, albedo.a,
        fsin_normal,
        roughness, metallic, ao, emissv,
        un_BRDF_LUT
    );

    vec3 result  = IDK_PBR_Dirlight(light, surface, fsin_fragpos);
         result *= dirlight_shadow_2(light, un_view, fsin_fragpos, surface.N);
         result += light.ambient.rgb * surface.albedo.rgb;

    fsout_radiance = vec4(result, albedo.a);
}
