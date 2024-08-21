#version 460 core

#extension GL_GOOGLE_include_directive: require
#extension GL_ARB_bindless_texture: require

#include "../include/storage.glsl"
#include "./water.glsl"


layout (location = 0) in vec3 vsin_pos;
layout (location = 1) in vec3 vsin_normal;
layout (location = 2) in vec3 vsin_tangent;
layout (location = 3) in vec2 vsin_texcoords;


out FS_in
{
    vec3 fragpos;
    vec2 texcoord;
    float dist;

} vsout;


const uint un_num_instances = 4;
// const float[2] CLIP_W = float[2]( 0.5*96.0, 128.0 );


layout (binding=0, r32f) readonly uniform image2D un_depth;



vec4 computeVertexPosition()
{
    IDK_Camera camera = IDK_UBO_cameras[0];
    vec2 cam_xz = camera.position.xz;

    const uint maxID = IDK_TERRAIN_NUM_CLIPS;
    uint clipID = gl_DrawID + gl_InstanceID;

    vec2 pos_xz = vsin_pos.xz;


    float max_scale   = IDK_TERRAIN_CLIP_W * pow(2, maxID);
    float clip_scale  = IDK_TERRAIN_CLIP_W * pow(2, clipID);
    float spacing     = clip_scale / IDK_TERRAIN_CLIP_W_VERTS;


    pos_xz = (clip_scale * pos_xz) + cam_xz;
    pos_xz = floor(pos_xz / spacing) * spacing;

    return vec4(pos_xz, 0.0, 0.0);
}

// vec4 computeVertexPosition()
// {
//     IDK_Camera camera = IDK_UBO_cameras[0];
//     vec2 cam_xz = camera.position.xz;

//     const uint maxID = IDK_TERRAIN_NUM_CLIPS;
//     uint clipID = gl_DrawID + gl_InstanceID;

//     vec2 pos_xz = vsin_pos.xz;

//     // if (clipID > 0 && clipID < maxID)
//     // {
//     //     if (abs(vsin_pos.x) == 0.5 || abs(vsin_pos.z) == 0.5)
//     //     {
//     //         clipID = clamp(clipID + 1, 0, maxID);
//     //         pos_xz *= 0.5;
//     //     }
//     // }

//     float max_scale   = IDK_TERRAIN_CLIP_W * pow(2, maxID);
//     float clip_scale  = IDK_TERRAIN_CLIP_W * pow(2, clipID);
//     float spacing     = clip_scale / 4.0;


//     pos_xz = (clip_scale * pos_xz) + cam_xz;
//     pos_xz = floor(pos_xz / spacing) * spacing;

//     return vec4(pos_xz, 0.0, 0.0);
// }



void main()
{
    IDK_Camera camera = IDK_UBO_cameras[0];
    const uint maxID = IDK_TERRAIN_NUM_CLIPS-1;
    float max_scale  = IDK_TERRAIN_CLIP_W * pow(2, maxID);

    vec2 local = computeVertexPosition().xy;
    vsout.fragpos = vec3(local.x, 0.0, local.y);
    vsout.texcoord = vsout.fragpos.xz;



    float water_height = 0.0;

    {
        float xscale = IDK_SSBO_Terrain.water_scale.x;
        float yscale = IDK_SSBO_Terrain.water_scale.y;
        float wscale = IDK_SSBO_Terrain.water_scale[3];

        float t  = IDK_GetTime();
        float dt = IDK_GetDeltaTime();
        vec2  pd = WaterComputeHeight(t-dt, vsout.texcoord.x/xscale, vsout.texcoord.y/xscale).yz;
            // pd /= xscale;
            pd *= yscale;

        vsout.texcoord += pd * wscale;


        water_height  = WaterComputeHeight(t, vsout.texcoord.x/xscale, vsout.texcoord.y/xscale)[0];
        water_height *= yscale;
    }

    {
        float xscale = IDK_SSBO_Terrain.scale.x;
        float yscale = IDK_SSBO_Terrain.scale.y;

        float water_level = IDK_SSBO_Terrain.transform[3].y + xscale * yscale * (IDK_SSBO_Terrain.water_pos.y);
        water_height += water_level;
    }


    vsout.fragpos.y = water_height;


    gl_Position = camera.P * camera.V * vec4(vsout.fragpos, 1.0);

}

