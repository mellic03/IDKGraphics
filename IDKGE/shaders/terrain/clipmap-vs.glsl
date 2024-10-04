#extension GL_GOOGLE_include_directive: require
#extension GL_ARB_bindless_texture: require

#include "../include/storage.glsl"
#include "../include/taa.glsl"
#include "./terrain.glsl"


layout (location = 0) in vec3 vsin_pos;
layout (location = 1) in vec3 vsin_normal;
layout (location = 2) in vec3 vsin_tangent;
layout (location = 3) in vec2 vsin_texcoords;


out FS_in
{
    vec3 fragpos;
    vec3 normal;
    vec3 tangent;
    vec2 texcoord;
    float height;
    float clipID;

    IDK_VelocityData vdata;
} vsout;



vec4 computeVertexPosition( float vert_scale )
{
    IDK_Camera camera = IDK_UBO_cameras[0];
    vec2 cam_xz = camera.position.xz;

    const uint maxID = IDK_TERRAIN_NUM_CLIPS;
    uint clipID = gl_DrawID + gl_InstanceID;
    vsout.clipID = clipID;

    vec2 pos_xz = vsin_pos.xz;
    pos_xz *= vert_scale;

    // if (clipID > 0)
    // {
    //     if (pos_xz.x >= 0.9999 || pos_xz.y >= 0.9999)
    //     {
    //         clipID = clamp(clipID + 1, 0, maxID);
    //         pos_xz *= 0.5;
    //     }
    // }

    float max_scale   = IDK_TERRAIN_CLIP_W * pow(2, maxID);
    float clip_scale  = IDK_TERRAIN_CLIP_W * pow(2, clipID);
    float spacing     = 2.0 * clip_scale / IDK_TERRAIN_CLIP_W_VERTS;


    pos_xz = ((clip_scale * pos_xz) + cam_xz);
    pos_xz = floor(pos_xz / spacing) * spacing;

    mat4 M = IDK_SSBO_Terrain.transform;
    float xscale = length(vec3(M[0]));

    vec2 uv = pos_xz / (xscale);
         uv += 0.5;

    return vec4(pos_xz, uv);
}




uniform uint un_light_id = 0;
uniform uint un_cascade  = 0;

void main()
{
    IDK_Camera camera = IDK_UBO_cameras[0]; 
    const uint maxID = IDK_TERRAIN_NUM_CLIPS-1;
    float max_scale  = IDK_TERRAIN_CLIP_W * pow(2, maxID);
    
    vec4 local = computeVertexPosition(4.0);
    vsout.fragpos = vec3(local.x, 0.0, local.y);
    vsout.texcoord = local.zw;

    float height = textureLod(IDK_SSBO_Terrain.height, vsout.texcoord, 0.0).r;
    // float detail = TerrainComputeDetail(vsout.fragpos.xz);

    vsout.height     = height * IDK_SSBO_Terrain.scale.x * IDK_SSBO_Terrain.scale.y;
    vsout.fragpos.y += vsout.height + IDK_SSBO_Terrain.transform[3].y;
    vsout.normal  = textureLod(IDK_SSBO_Terrain.nmap, vsout.texcoord, 0.0).rgb;
    vsout.tangent = vsin_tangent;

    // vsout.fragpos += vsout.normal * detail;

    #ifndef CLIPMAP_SHADOW
        vsout.vdata = PackVData(camera, vsout.fragpos, vsout.fragpos);
        gl_Position = camera.P * camera.V * vec4(vsout.fragpos, 1.0);

    #else
        IDK_Dirlight light = IDK_UBO_dirlights[un_light_id];
        mat4 T = (un_cascade <= 3) ? light.transforms[un_cascade] : light.transform;
        gl_Position = T * vec4(vsout.fragpos, 1.0);

    #endif

}
