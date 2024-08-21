#version 460 core

#extension GL_GOOGLE_include_directive: require

#include "../include/storage.glsl"
#include "../include/util.glsl"
#include "./terrain.glsl"
#include "./water.glsl"


layout (location = 0) out vec4 fsout_albedo;
layout (location = 1) out vec3 fsout_normal;
layout (location = 2) out vec4 fsout_pbr;


in FS_in
{
    vec3 fragpos;
    vec3 normal;
    vec3 tangent;
    vec2 texcoord;
    float clipID;
} fsin;



vec3 computeNormal()
{
    vec3 fdx = vec3(dFdx(fsin.fragpos.x), dFdx(fsin.fragpos.y), dFdx(fsin.fragpos.z));
    vec3 fdy = vec3(dFdy(fsin.fragpos.x), dFdy(fsin.fragpos.y), dFdy(fsin.fragpos.z));

    return normalize(cross(fdx, fdy));
}


// vec2 rotateUV(vec2 uv, float rotation)
// {
//     float mid = 0.5;
//     return vec2(
//         cos(rotation) * (uv.x - mid) + sin(rotation) * (uv.y - mid) + mid,
//         cos(rotation) * (uv.y - mid) - sin(rotation) * (uv.x - mid) + mid
//     );
// }



struct Material
{
    vec4 diff;
    vec3 norm;
    vec3 arm;
};





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



Material packMaterial( uint idx, vec3 N, float scale )
{
    Material mat;

    {
        mat.diff = vec4(0.0);

        vec3 W = abs(N);
            W /= (W.x + W.y + W.z);

        vec3 S = scale * fsin.fragpos;

        mat.diff += W.z * texture(IDK_SSBO_Terrain.diff, vec3(S.xy, idx));
        mat.diff += W.x * texture(IDK_SSBO_Terrain.diff, vec3(S.yz, idx));
        mat.diff += W.y * texture(IDK_SSBO_Terrain.diff, vec3(S.xz, idx));
    }


    {
        mat.arm = vec3(0.0);

        vec3 W = abs(N);
            W /= (W.x + W.y + W.z);

        vec3 S = scale * fsin.fragpos;

        mat.arm += W.z * texture(IDK_SSBO_Terrain.arm, vec3(S.xy, idx)).rgb;
        mat.arm += W.x * texture(IDK_SSBO_Terrain.arm, vec3(S.yz, idx)).rgb;
        mat.arm += W.y * texture(IDK_SSBO_Terrain.arm, vec3(S.xz, idx)).rgb;
    }

    // mat.diff = texture(IDK_SSBO_Terrain.diff, vec3(0.12*fsin.fragpos.xz, idx));
    mat.norm = texture(IDK_SSBO_Terrain.norm, vec3(scale*fsin.texcoord, idx)).rgb * 2.0 - 1.0;
    mat.arm  = texture(IDK_SSBO_Terrain.arm,  vec3(scale*fsin.texcoord, idx)).rgb;

    return mat;
}


Material mixMaterial( Material A, Material B, float alpha)
{
    Material AB;

    AB.diff = mix(A.diff, B.diff, alpha);
    AB.norm = mix(A.norm, B.norm, alpha);
    AB.arm  = mix(A.arm,  B.arm,  alpha);

    return AB;
}

layout (binding=0, r32f) writeonly uniform image2D un_depth;




float TerrainComputeWorldY( vec2 world_xz )
{
    mat4 T = IDK_SSBO_Terrain.transform;

    float xscale = (mat3(T) * vec3(1.0)).x;
    float yscale = IDK_SSBO_Terrain.scale.y;

    vec2  min_xz  = T[3].xz + xscale * vec2(-0.5);
    vec2  max_xz  = T[3].xz + xscale * vec2(+0.5);

    vec2 texcoord = (world_xz - min_xz) / (max_xz - min_xz);
         texcoord = clamp(texcoord, 0.0, 1.0);

    return T[3].y + TerrainSampleHeight(texcoord).y;
}


vec3 computeNormal2()
{
    mat4 T = IDK_SSBO_Terrain.transform;
    float xscale = (mat3(T) * vec3(1.0)).x;

    float delta = 0.5; // xscale / textureSize(IDK_SSBO_Terrain.height, 0).x;

    float left  = TerrainComputeWorldY(fsin.fragpos.xz - vec2(delta, 0.0));
    float right = TerrainComputeWorldY(fsin.fragpos.xz + vec2(delta, 0.0));
    float up    = TerrainComputeWorldY(fsin.fragpos.xz - vec2(0.0, delta));
    float down  = TerrainComputeWorldY(fsin.fragpos.xz + vec2(0.0, delta));

    vec3 L = vec3(-delta, left,  0.0);
    vec3 R = vec3(+delta, right, 0.0);
    vec3 U = vec3(0.0,    up,    -delta);
    vec3 D = vec3(0.0,    down,  +delta);

    vec3 N = normalize(cross(R-L, U-D));
        //  N = vec3(N.x, -N.y, N.z);
    
    return N;
}














void main()
{
    IDK_Camera cam = IDK_UBO_cameras[0];

    vec2 texcoord = IDK_WorldToUV(fsin.fragpos, cam.P * cam.V).xy;

    vec3 normal = textureLod(IDK_SSBO_Terrain.nmap, fsin.texcoord, 0.0).rgb;
         normal = normalize(normal);

    // vec3 normal = normalize(fsin.normal);

    // if (texcoord.x > 0.5)
    // {
    //     normal = computeNormal();
    // }


    vec4 albedo = vec4(0.0, 0.0, 0.0, 1.0);

    float scale0 = IDK_SSBO_Terrain.texscale[0][1];
    float scale1 = IDK_SSBO_Terrain.texscale[0][2];
    float scale2 = IDK_SSBO_Terrain.texscale[0][3];

    Material grass = packMaterial(0, normal, scale0);
    Material sand0 = packMaterial(1, normal, scale1);
    Material rock1 = packMaterial(2, normal, scale2);
    Material rock2 = packMaterial(3, normal, scale2);
    rock2.diff *= 0.5;

    Material snow  = packMaterial(4, normal, scale0);

    Material ground;
    Material result;


    {
        float lo = IDK_SSBO_Terrain.height_blend[0];
        float hi = IDK_SSBO_Terrain.height_blend[1];

        float alpha = textureLod(IDK_SSBO_Terrain.height, fsin.texcoord, 0.0).r;
              alpha = clamp(alpha, 0.0, 1.0);
              alpha = smoothstep(lo, hi, alpha);

        ground = mixMaterial(sand0, grass, alpha);
    }

    {
        float lo = IDK_SSBO_Terrain.slope_blend[0];
        float hi = IDK_SSBO_Terrain.slope_blend[1];

        float alpha = smoothstep(lo, hi, 1.0 - abs(normal.y));

        result = mixMaterial(ground, rock2, alpha);
    }

    float ao    = result.arm[0];
    float rough = result.arm[1];
    float metal = result.arm[2];


    float dist  = distance(fsin.fragpos, cam.position.xyz);
    // vec2  texcoord = IDK_WorldToUV(fsin.fragpos, cam.P * cam.V).xy;
    ivec2 texel    = ivec2(texcoord * vec2(cam.width, cam.height));
    imageStore(un_depth, texel, vec4(dist));


    vec4 color = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 shallow_color = IDK_SSBO_Terrain.water_color[0];
    vec4 deep_color    = IDK_SSBO_Terrain.water_color[1]; 

    float terrain_height  = textureLod(IDK_SSBO_Terrain.height, fsin.texcoord, 0.0).r;

    // if (terrain_height < IDK_SSBO_Terrain.water_pos.y)
    // {
    //     color = shallow_color;
    // }

    // if (terrain_height < 0.25)
    // {
    //     color.r = 1.0;
    // }

    // else if (terrain_height < 0.5)
    // {
    //     color.g = 1.0;
    // }

    // else if (terrain_height < 0.75)
    // {
    //     color.b = 1.0;
    // }

    // else if (terrain_height < 1.0)
    // {
    //     color.rgb += 1.0;
    // }

    // else if (terrain_height < 1.25)
    // {
    //     color.rb += 1.0;
    // }

    //       terrain_height *= IDK_SSBO_Terrain.scale.x * IDK_SSBO_Terrain.scale.y;

    // float water_height;


    // {
    //     vec3 vsout_fragpos  = 512.0 * vec3(fsin.texcoord.x, 0.0, fsin.texcoord.y);
    //          vsout_fragpos += IDK_SSBO_Terrain.transform[3].xyz;

    //     vec2 vsout_texcoord  = vsout_fragpos.xz;
    //     vsout_texcoord /= IDK_SSBO_Terrain.water_scale[0];

    //     float t  = IDK_GetTime();
    //     float dt = IDK_GetDeltaTime();
    //     vec2  pd = WaterComputeHeight(t-dt, vsout_texcoord.x, vsout_texcoord.y).yz;
    //         pd /= IDK_SSBO_Terrain.water_scale[0];
    //         pd *= IDK_SSBO_Terrain.water_scale[1];

    //     vsout_texcoord += pd * IDK_SSBO_Terrain.water_scale[3];

    //     float water_level   = IDK_SSBO_Terrain.scale.x * IDK_SSBO_Terrain.water_scale[1] * (IDK_SSBO_Terrain.water_pos.y);
    //     water_height  = WaterComputeHeight(t, vsout_texcoord.x, vsout_texcoord.y)[0];
    //     water_height *= IDK_SSBO_Terrain.water_scale[1];
    // }


    // if (terrain_height < water_height)
    // {
    //     result.diff.rgb = vec3(0.2, 0.5, 1.0);
    // }



    fsout_albedo = result.diff;
    fsout_normal = normal;
    fsout_pbr    = vec4(rough, metal, ao, 0.0);
}
