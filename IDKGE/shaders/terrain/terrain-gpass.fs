#version 460 core

#extension GL_GOOGLE_include_directive: require

#include "../include/storage.glsl"
#include "../include/util.glsl"
#include "./terrain.glsl"


layout (location = 0) out vec4 fsout_albedo;
layout (location = 1) out vec3 fsout_normal;
layout (location = 2) out vec4 fsout_pbr;


in FS_in
{
    vec3 fragpos;
    vec3 normal;
    vec3 tangent;
    vec2 texcoord;
    float tesslevel;
    float height;
    mat3 TBN;

} fsin;




// vec4 triplanar( uint idx, vec3 N )
// {
//     vec4 result = vec4(0.0);

//     vec3 W = abs(N);
//          W /= (W.x + W.y + W.z);

//     vec3 S = fsin.fragpos * IDK_SSBO_Terrain.texscale[0][idx/4];

//     result += W.z * texture(IDK_SSBO_Terrain.textures[idx], S.xy);
//     result += W.x * texture(IDK_SSBO_Terrain.textures[idx], S.yz);
//     result += W.y * texture(IDK_SSBO_Terrain.textures[idx], S.xz);

//     return result;
// }


vec3 computeNormal()
{
    vec3 fdx = vec3(dFdx(fsin.fragpos.x), dFdx(fsin.fragpos.y), dFdx(fsin.fragpos.z));
    vec3 fdy = vec3(dFdy(fsin.fragpos.x), dFdy(fsin.fragpos.y), dFdy(fsin.fragpos.z));

    return normalize(cross(fdx, fdy));
}


vec2 rotateUV(vec2 uv, float rotation)
{
    float mid = 0.5;
    return vec2(
        cos(rotation) * (uv.x - mid) + sin(rotation) * (uv.y - mid) + mid,
        cos(rotation) * (uv.y - mid) - sin(rotation) * (uv.x - mid) + mid
    );
}


// vec4 biplanar( uint idx, vec3 pos, float factor )
// {
//     vec2 uv0 = IDK_SSBO_Terrain.texscale[0][idx/4] * (pos.xz);
//     vec2 uv1 = IDK_SSBO_Terrain.texscale[0][idx/4] * vec2(0.5*(pos.x+pos.z), pos.y);

//     float n0 = (IDK_PerlinNoise(uv0, 4).r) * 2.0 * 3.14159;
//     float n1 = (IDK_PerlinNoise(uv1, 4).r) * 2.0 * 3.14159;

//     // uv0 = rotateUV(uv0, n0);
//     // uv1 = rotateUV(uv1, n1);

//     vec4 sample00 = texture(IDK_SSBO_Terrain.textures[idx], uv0);
//     vec4 sample01 = texture(IDK_SSBO_Terrain.textures[idx], n0*uv0);
//     vec4 sample10 = texture(IDK_SSBO_Terrain.textures[idx], uv1);
//     vec4 sample11 = texture(IDK_SSBO_Terrain.textures[idx], n1*uv1);

//     vec4 sample0 = 0.5 * (sample00 + sample01);
//     vec4 sample1 = 0.5 * (sample10 + sample11);

//     return mix(sample0, sample1, factor);
// }

// vec4 biplanar(  idx, vec3 pos, float factor )
// {
//     vec2 uv0 = IDK_SSBO_Terrain.texscale[0][idx/4] * (pos.xz);
//     vec2 uv1 = IDK_SSBO_Terrain.texscale[0][idx/4] * vec2(0.5*(pos.x+pos.z), pos.y);

//     float n0 = (IDK_PerlinNoise(uv0, 4).r) * 2.0 * 3.14159;
//     float n1 = (IDK_PerlinNoise(uv1, 4).r) * 2.0 * 3.14159;

//     // uv0 = rotateUV(uv0, n0);
//     // uv1 = rotateUV(uv1, n1);

//     vec4 sample00 = texture(IDK_SSBO_Terrain.textures[idx], uv0);
//     vec4 sample01 = texture(IDK_SSBO_Terrain.textures[idx], n0*uv0);
//     vec4 sample10 = texture(IDK_SSBO_Terrain.textures[idx], uv1);
//     vec4 sample11 = texture(IDK_SSBO_Terrain.textures[idx], n1*uv1);

//     vec4 sample0 = 0.5 * (sample00 + sample01);
//     vec4 sample1 = 0.5 * (sample10 + sample11);

//     return mix(sample0, sample1, factor);
// }



void main()
{
    IDK_Camera cam = IDK_UBO_cameras[0];
    vec4 proj = cam.P * cam.V * vec4(fsin.fragpos, 1.0);
         proj.xy /= proj.w;
         proj.xy = proj.xy * 0.5 + 0.5;

    float xscale = (mat3(IDK_SSBO_Terrain.transform) * vec3(1.0)).x;
    float yscale = IDK_SSBO_Terrain.scale.y;
    float nscale = IDK_SSBO_Terrain.scale[3];

    vec3  result = TerrainComputeHeight(fsin.texcoord);
    float height = yscale * result[0];

    float dX = result[1];
    float dZ = result[2];


    vec3 normal;

    // if (proj.x > 0.5)
    // {
    //     vec3  result = TerrainComputeHeight(fsin.texcoord);

    //     float dX = result[1];
    //     float dZ = result[2];

    //     vec3 T = vec3(1.0, dX, 0.0);
    //     vec3 B = vec3(0.0, dZ, 1.0);

    //     normal = -normalize(cross(T, B));
    //     normal.y *= nscale;
    // }

    // else
    {
        normal = computeNormal();
    }

    float factor = 1.0 - abs(normal.y);

    float scale0 = IDK_SSBO_Terrain.texscale[0][1];
    float scale1 = IDK_SSBO_Terrain.texscale[0][2];
    float scale2 = IDK_SSBO_Terrain.texscale[0][3];

    vec4 grass1 = texture(IDK_SSBO_Terrain.diff, vec3(scale0*fsin.texcoord, 0));
    vec4 grass2 = texture(IDK_SSBO_Terrain.diff, vec3(0.23*fsin.texcoord, 0));
    vec4 grass  = grass1 + grass2;

    vec4 rock0 = texture(IDK_SSBO_Terrain.diff, vec3(fsin.texcoord, 1));
    vec4 rock1 = texture(IDK_SSBO_Terrain.diff, vec3(fsin.texcoord, 2));
    vec4 rock2  = texture(IDK_SSBO_Terrain.diff, vec3(scale1*fsin.texcoord, 3));

    vec4 snow  = texture(IDK_SSBO_Terrain.diff, vec3(fsin.texcoord, 4));

    // vec4 snow  = biplanar(4*3+0, fsin.fragpos, factor);
    // vec4 rock  = biplanar(4*2+0, fsin.fragpos, 1.0 - factor);

    vec4 albedo = vec4(0.0, 0.0, 0.0, 1.0);

    // {
    //     float lo = IDK_SSBO_Terrain.height_blend[0];
    //     float hi = IDK_SSBO_Terrain.height_blend[1];

    //     float alpha = smoothstep(lo, hi, clamp(fsin.height, 0.0, 1.0));

    //     albedo = mix(grass, snow, alpha);
    // }


    {
        float lo = IDK_SSBO_Terrain.slope_blend[0];
        float hi = IDK_SSBO_Terrain.slope_blend[1];

        float alpha = smoothstep(lo, hi, 1.0 - abs(normal.y));
        albedo = mix(grass, rock2, alpha);
    }


    fsout_albedo = vec4(albedo.rgb, 1.0);
    fsout_normal = normalize(normal);
    fsout_pbr    = vec4(1.0, 0.7, 0.0, 0.0);
    // fsout_pbr    = vec4(ao, roughness, metallic, 0.0);
}
