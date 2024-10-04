#version 460 core

#extension GL_GOOGLE_include_directive: require
#include "../include/storage.glsl"
#include "../include/util.glsl"
#include "../include/noise.glsl"
#include "../include/time.glsl"
#include "../include/pbr.glsl"
#include "../include/volumetric.glsl"
#include "../include/taa.glsl"

#include "../generative/worley.glsl"


layout (location = 0) out vec4 fsout_frag_color;

in vec3 fsin_fragpos;
flat in uint lightID;


uniform int   un_samples        = 16;
uniform int   un_samples_sun    = 2;
uniform float un_intensity      = 1.0;

uniform float un_height_offset  = 0.0;
uniform float un_height_falloff = 1.0;
uniform float un_scatter_coeff  = 1.0;
uniform float un_absorb_coeff   = 3.0;

uniform float un_worley_amp     = 1.0;
uniform float un_worley_wav     = 0.02;


uniform sampler2D un_color;
uniform sampler2D un_fragdepth;
uniform sampler2DArrayShadow un_shadowmap;




float dirlight_shadow_2( IDK_Camera camera, IDK_Dirlight light, mat4 V, vec3 position )
{
    const float[4] biases = float[4]( 0.005, 0.006, 0.008, 0.016 );

    vec3 L = normalize(-light.direction.xyz);

    vec3 fragpos_viewspace = (V * vec4(position, 1.0)).xyz;
    vec4 cascade_depths = light.cascades;
    vec4 res     = step(cascade_depths, vec4(abs(fragpos_viewspace.z)));
    int  cascade = int(res.x + res.y + res.z + res.w);
         cascade = clamp(cascade, 0, 4);

    vec4  fragpos_lightspace = light.transforms[cascade] * vec4(position, 1.0);
    vec3  projCoords = fragpos_lightspace.xyz / fragpos_lightspace.w;
          projCoords = projCoords * 0.5 + 0.5;

    float frag_depth = abs(projCoords.z);
    float bias = 0.005; // biases[cascade] * max(dot(N, L), 0.01);
        //   bias /= (cascade_depths[cascade] * 2.0);

    vec2  texelSize = 1.0 / textureSize(un_shadowmap, 0).xy;

    return texture(un_shadowmap, vec4(projCoords.xy, cascade, frag_depth-bias)).r;
}


float IDK_AtmosphericDensity( float altitude, float max_altitude )
{
    float base = 4.0;

    float d = 1.0 - clamp(altitude / max_altitude, 0.0, 1.0);
    return d * base * exp(-altitude / max_altitude);
}


vec3 IDK_AtmosphericMie( float cosTheta )
{
    vec3 base = vec3(210e-1);
    return base * IDK_MieScattering(cosTheta);
}



vec3 IDK_AtmosphericRayleigh( float cosTheta )
{
    vec3 base = vec3(33.1e-2, 13.5e-2, 5.8e-2);
    return base * IDK_RayleighScattering(cosTheta);
}




float SampleWorley( float a, float w, float t, vec3 rp )
{
    const float hw = 12.5;
    const float hh = 6.0;
    const float hd = 8.0;

    vec3 rpp = clamp(rp, vec3(-4.25-hw, 8, 16.0-hd), vec3(-4.25+hw, 23, 16.0+hd));
    float d = distance(rp, rpp);

    return (rp == rpp) ? 1.0 : 0.0;

    // float d = distance(rp, vec3(0, 28, 0));
        //   d = clamp((d-3.0) / 4.0, 0.0, 1.0);
// 
    // return 1.0 - d;

    // return (d < 4.0) ? 1.0 : 0.0;
}



float densityAltitude( float y )
{
    return exp(-(y - un_height_offset) * un_height_falloff);

    // float altitude = y - un_height_offset;
    // float d = 1.0 - clamp(altitude / un_height_falloff, 0.0, 1.0);
    // return d * exp(-altitude * un_height_falloff);
}


float FractalWorley( float a, float w, float t, vec3 rp )
{
    float result = 0.0;

    for (int i=0; i<4; i++)
    {
        result += a * (worley(w*(rp + t*vec3(1, 0, 1))));
        a *= 0.5;
        w *= 2.0;
    }

    // result += a * IDK_WorleyNoise3D(w*(rp + t)).r;
    // result += 0.5*a * IDK_WorleyNoise3D(2.0*w*(rp + t)).r;
    // return result*result;

    result *= densityAltitude(rp.y);

    return pow(result, un_worley_amp);
}



float ComputeSunTransmittance( IDK_Dirlight light, IDK_Camera camera, float oa, float t, vec3 rp, vec3 L )
{
    float step_size = 0.5;
    float density = 0.0;

    for (int i=0; i<un_samples_sun; i++)
    {
        rp += step_size * L;

        float d  = 4.0 * FractalWorley(un_worley_amp, un_worley_wav, t, rp) * step_size;
            //   d *= dirlight_shadow(camera, light, camera.V, rp) * step_size;
        density += max(d, 0.0);
    }

    float re = exp(-oa * density);
    float a = 0.0;

    return a + re * (1.0 - a);
}



// vec4 raymarch( vec3 ray_start, vec3 ray_end, int samples, vec3 L, vec2 texel, vec4 result )
// {
//     IDK_Camera   camera = IDK_UBO_cameras[0];
//     IDK_Dirlight light  = IDK_UBO_dirlights[lightID];

//     vec3  rp = ray_start; // mix(viewpos, fragpos, alpha_start);
//     vec3  re = ray_end; // mix(viewpos, fragpos, alpha_end);
//     vec3  rd = normalize(re - rp);

//     float rl = distance(rp, re);
//     float rs = rl / float(samples);

//     vec3 offset = IDK_BlueNoiseTexel(ivec2(texel)).rgb * 2.0 - 1.0;
//         //  offset = (inverse(camera.V) * vec4(offset, 0.0)).xyz;

//     rp += 2.0 * offset * rs * rd;
//     rd = normalize(re - rp);
//     rl = distance(rp, re);
//     rs = rl / float(samples);

//     float t  =  IDK_GetTime();
//     float oa = 0.1 * un_absorb_coeff;
//     float os = 0.1 * un_scatter_coeff;
//     float oe = oa + os;

//     float density = 0.0;
//     float transmittance = 1.0;
//     vec3 illumination = vec3(0.0);
//     float rayleigh      = IDK_RayleighScattering(max(dot(rd, L), 0.0));
//     float mie           = IDK_MieScattering(max(dot(rd, L), 0.0));
//     vec3  aRayleigh     = IDK_AtmosphericRayleigh(max(dot(rd, L), 0.0));
//     vec3  aMie          = IDK_AtmosphericMie(max(dot(rd, L), 0.0));

//     float shadow = 0.0;

//     for (int i=0; i<samples; i++)
//     {
//         rp += rs * rd;

//         shadow += dirlight_shadow_2(camera, light, camera.V, rp);
//         density = rs * FractalWorley(un_worley_amp, un_worley_wav, t, rp);

//         if (density > 0.0)
//         {
//             float lt = ComputeSunTransmittance(light, camera, oa, t, rp, L);
//             illumination += density * rs * transmittance * lt * rayleigh;
//             transmittance *= exp(-oe * density * rs);
//         }

//         // transmittance *= exp(-oe * density);
//         // illumination += rs * light.diffuse.rgb;
//     }

//     return vec4((1.0-shadow)*illumination, transmittance);

//     // return un_intensity * vec4(rayleigh * illumination, clamp(density, 0.0, 1.0));

// }


// void main()
// {
//     IDK_Camera   camera = IDK_UBO_cameras[0];
//     IDK_Dirlight light  = IDK_UBO_dirlights[lightID];


//     vec4 proj_uv  = camera.P * camera.V * vec4(fsin_fragpos, 1.0);
//     vec2 texcoord = (proj_uv.xy / proj_uv.w) * 0.5 + 0.5;

//     // vec2 texcoord = IDK_WorldToUV(fsin_fragpos, camera.P * camera.V).xy;
//     vec2 texel    = texcoord * vec2(camera.width, camera.height);
//     vec3 worldpos = IDK_WorldFromDepth(un_fragdepth, texcoord, camera.P, camera.V);


//     vec3 L  = -light.direction.xyz;
//     vec3 rd = normalize(worldpos - camera.position.xyz);
//     vec3 ro = camera.position.xyz;
//     vec3 rp = ro;

//     float dist0 = min(distance(rp, worldpos), 512.0);

//     float sample_alpha = 1.0;

//     vec3 ro0 = ro + 0.0;
//     vec3 re0 = ro + dist0*rd;
//     int  sm0 = int(sample_alpha * un_samples);

//     // // if (distance(camera.position.xyz, worldpos) >= 0.95 * camera.far)
//     // {
//     //     vec3 pos = ro - vec3(0.0, un_height_offset, 0.0);

//     //     float alpha = VolumetricFogAlpha(un_height_falloff, un_intensity, pos, rd, distance(rp, worldpos));
//     //     vec3  color = VolumetricFogColor(light, rd);

//     //     // vec3 result = bad_atmospherics(ray_dir, light.direction.xyz, light);
//     //     fsout_frag_color = vec4(color, alpha);
//     //     // fsout_frag_color = vec4(0.0, 0.0, 0.0, 1.0);
//     //     // return;
//     // }


//     vec4 result = vec4(0.0, 0.0, 0.0, 1.0);
//     result = raymarch(ro0, re0, sm0, L, texel, result);

//     vec3 bg = textureLod(un_color, texcoord, 0.0).rgb;
//     vec3 cg = result.rgb * light.diffuse.rgb;
//     vec3 col = bg * result.w + cg;

//     fsout_frag_color = vec4(col, 1.0);

//     // fsout_frag_color = result;

// }



void main()
{
    IDK_Camera   camera = IDK_UBO_cameras[0];
    IDK_Dirlight light  = IDK_UBO_dirlights[lightID];


    vec4 proj_uv  = camera.P * camera.V * vec4(fsin_fragpos, 1.0);
    vec2 texcoord = (proj_uv.xy / proj_uv.w) * 0.5 + 0.5;

    // vec2 texcoord = IDK_WorldToUV(fsin_fragpos, camera.P * camera.V).xy;
    vec2 texel    = texcoord * vec2(camera.width, camera.height);
    vec3 worldpos = IDK_WorldFromDepth(un_fragdepth, texcoord, camera.P, camera.V);


    vec3  L  = -light.direction.xyz;
    vec3  rd = normalize(worldpos - camera.position.xyz);
    vec3  ro = camera.position.xyz;
    vec3  rp = ro;
    float rs = distance(worldpos, ro) / un_samples;

    vec3 result = vec3(0.0);

    for (int i=0; i<un_samples; i++)
    {
        rp += rs*rd;

        float density = FractalWorley(un_worley_amp, un_worley_wav, IDK_GetTime(), rp);
        float shadow  = dirlight_shadow_2(camera, light, camera.V, worldpos);
        vec3  ambient = 0.025*light.diffuse.rgb;
        vec3  diffuse = 0.25*light.diffuse.rgb;

        result += rs * density * (shadow*diffuse + ambient);
    }

    result /= distance(worldpos, ro);

    // vec3 bg = textureLod(un_color, texcoord, 0.0).rgb;
    // vec3 cg = result.rgb * light.diffuse.rgb;
    // vec3 col = bg + cg;

    fsout_frag_color = vec4(result, 1.0);


    // fsout_frag_color = result;

}


