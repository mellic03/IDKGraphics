#version 460 core
#extension GL_GOOGLE_include_directive: require

#include "clipmap-fs.glsl"


// #version 460 core

// #extension GL_GOOGLE_include_directive: require

// #include "../include/storage.glsl"
// #include "../include/util.glsl"
// #include "../include/taa.glsl"
// #include "./terrain.glsl"


// layout (location = 0) out vec4 fsout_albedo;
// layout (location = 1) out vec3 fsout_normal;
// layout (location = 2) out vec4 fsout_pbr;
// layout (location = 3) out vec2 fsout_vel;


// in FS_in
// {
//     vec3 fragpos;
//     vec3 normal;
//     vec3 tangent;
//     vec2 texcoord;
//     float height;
//     float clipID;

//     IDK_VelocityData vdata;
// } fsin;



// vec3 computeNormal()
// {
//     vec3 fdx = vec3(dFdx(fsin.fragpos.x), dFdx(fsin.fragpos.y), dFdx(fsin.fragpos.z));
//     vec3 fdy = vec3(dFdy(fsin.fragpos.x), dFdy(fsin.fragpos.y), dFdy(fsin.fragpos.z));

//     return normalize(cross(fdx, fdy));
// }




// struct Material
// {
//     vec4 diff;
//     vec3 norm;
//     vec3 arm;
// };




// // vec4 triplanar( uint idx, vec3 N )
// // {
// //     vec4 result = vec4(0.0);

// //     vec3 W = abs(N);
// //          W /= (W.x + W.y + W.z);

// //     vec3 S = fsin.fragpos * IDK_SSBO_Terrain.texscale[0][idx/4];

// //     result += W.z * texture(IDK_SSBO_Terrain.textures[idx], S.xy);
// //     result += W.x * texture(IDK_SSBO_Terrain.textures[idx], S.yz);
// //     result += W.y * texture(IDK_SSBO_Terrain.textures[idx], S.xz);

// //     return result;
// // }



// Material packMaterial( uint idx, vec3 N, float scale )
// {
//     Material mat;

//     vec3 W = abs(N);
//          W /= (W.x + W.y + W.z);

//     vec3 S = scale * fsin.fragpos;

//     // {
//     //     mat.diff = vec4(0.0);
//     //     mat.diff += W.z * texture(IDK_SSBO_Terrain.diff, vec3(S.xy, idx));
//     //     mat.diff += W.x * texture(IDK_SSBO_Terrain.diff, vec3(S.yz, idx));
//     //     mat.diff += W.y * texture(IDK_SSBO_Terrain.diff, vec3(S.xz, idx));
//     // }


//     // {
//     //     mat.arm = vec3(0.0);
//     //     mat.arm += W.z * texture(IDK_SSBO_Terrain.arm, vec3(S.xy, idx)).rgb;
//     //     mat.arm += W.x * texture(IDK_SSBO_Terrain.arm, vec3(S.yz, idx)).rgb;
//     //     mat.arm += W.y * texture(IDK_SSBO_Terrain.arm, vec3(S.xz, idx)).rgb;
//     // }

//     mat.diff = texture(IDK_SSBO_Terrain.diff, vec3(scale*fsin.fragpos.xz, idx));
//     mat.norm = texture(IDK_SSBO_Terrain.norm, vec3(scale*fsin.fragpos.xz, idx)).rgb * 2.0 - 1.0;
//     mat.arm  = texture(IDK_SSBO_Terrain.arm,  vec3(scale*fsin.fragpos.xz, idx)).rgb;

//     return mat;
// }


// Material packMaterial( uint idx, vec3 N, float scale, float theta )
// {
//     Material mat;

//     vec3 W = abs(N);
//          W /= (W.x + W.y + W.z);

//     vec3 S  = scale * fsin.fragpos;
//     vec2 uv = rotateUV(scale*fsin.fragpos.xz, theta);

//     mat.diff = texture(IDK_SSBO_Terrain.diff, vec3(uv, idx));
//     mat.norm = texture(IDK_SSBO_Terrain.norm, vec3(uv, idx)).rgb * 2.0 - 1.0;
//     mat.arm  = texture(IDK_SSBO_Terrain.arm,  vec3(uv, idx)).rgb;

//     return mat;
// }



// Material mixMaterial( Material A, Material B, float alpha)
// {
//     Material AB;

//     AB.diff = mix(A.diff, B.diff, alpha);
//     AB.norm = mix(A.norm, B.norm, alpha);
//     AB.arm  = mix(A.arm,  B.arm,  alpha);

//     return AB;
// }


// vec3 color_mix( float h, float lo, float hi, vec3 clo, vec3 chi )
// {
//     h = clamp(h, lo, hi);

//     float a = (h - lo) / (hi - lo);
//     return mix(clo, chi, a);
// }



// vec4 color_thing( float h )
// {
//     vec4 result = vec4(0.0, 0.0, 0.0, 1.0);

//     float[5] heights = float[5]
//     (
//         0.0, 0.25, 0.5, 0.75, 1.0
//     );

//     vec3[5] colors = vec3[5]
//     (
//         vec3(0.5, 0, 0),
//         vec3(1, 0, 0),
//         vec3(0.5, 0.5, 0),
//         vec3(1, 1, 0),
//         vec3(0, 1, 0)
//     );

//     for (int i=1; i<5; i++)
//     {
//         if (h <= heights[i])
//         {
//             result.rgb = color_mix(h, heights[i-1], heights[i], colors[i-1], colors[i]);
//             return result;
//         }
//     }

//     return result;
// }



// Material Bruh( int idx, vec3 N )
// {
//     float scale     = 2.0;
//     float scale4[4] = float[4]( 0.25, 0.1, 0.05, 0.01 );
//     float theta4[4] = float[4]( 0.25, 0.5, 0.75, 1.01 );


//     Material mat0123[4];
    
//     for (int i=0; i<4; i++)
//     {
//         mat0123[i] = packMaterial(idx, N, scale*scale4[i], theta4[i]);
//     }

//     Material mat01 = mixMaterial(mat0123[0], mat0123[1], 0.5);
//     Material mat10 = mixMaterial(mat0123[2], mat0123[3], 0.5);
//     Material mat   = mixMaterial(mat01,      mat10,      0.5);

//     return mat;
// }



// #ifndef CLIPMAP_SHADOW

// void main()
// {
//     IDK_Camera cam = IDK_UBO_cameras[0];
//     vec2 texcoord = IDK_WorldToUV(fsin.fragpos, cam.P * cam.V).xy;

//     vec3 normal = normalize(fsin.normal);

//     if (texcoord.x > 0.5)
//     {
//         normal = computeNormal();
//     }


//     vec4 albedo = vec4(0.0, 0.0, 0.0, 1.0);

//     float grassness = textureLod(IDK_SSBO_Terrain.height, fsin.texcoord, 0.0).g;
//           grassness = clamp(grassness, 0.0, 1.0);

//     Material grass = Bruh(0, normal);
//     Material sand0 = Bruh(1, normal);
//     Material rock2 = Bruh(2, normal);
//     Material rock1 = Bruh(3, normal);
//     Material snow  = Bruh(4, normal);

//     Material ground;
//     Material result;

//     {
//         float lo = IDK_SSBO_Terrain.height_blend[0];
//         float hi = IDK_SSBO_Terrain.height_blend[1];

//         float alpha = fsin.height;
//               alpha = clamp(alpha, 0.0, 1.0);
//               alpha = smoothstep(lo, hi, alpha);

//         ground = mixMaterial(sand0, grass, alpha*grassness);
//     }

//     {
//         float lo = IDK_SSBO_Terrain.slope_blend[0];
//         float hi = IDK_SSBO_Terrain.slope_blend[1];

//         float alpha = smoothstep(lo, hi, 1.0 - abs(normal.y));

//         result = mixMaterial(ground, rock2, alpha);
//     }

//     float ao    = result.arm[0];
//     float rough = result.arm[1];
//     float metal = result.arm[2];


//     float dist  = distance(fsin.fragpos, cam.position.xyz);
//     ivec2 texel    = ivec2(texcoord * vec2(cam.width, cam.height));

//     // // if (texcoord.x > 0.5)
//     // {
//         // fsout_albedo = color_thing(fsin.height);
//     //     // fsout_albedo = vec4(dist / 128.0);
//     // }

//     fsout_albedo = result.diff;
//     fsout_normal = normal;
//     fsout_pbr    = vec4(rough, metal, ao, 0.0);
//     fsout_vel    = PackVelocity(fsin.vdata);

// }

// #else
// void main()
// {

// }
// #endif
