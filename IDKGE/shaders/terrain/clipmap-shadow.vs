#version 460 core

#extension GL_GOOGLE_include_directive: require
#extension GL_ARB_bindless_texture: require


#include "../include/storage.glsl"
#include "./terrain.glsl"


layout (location = 0) in vec3 vsin_pos;
layout (location = 1) in vec3 vsin_normal;
layout (location = 2) in vec3 vsin_tangent;
layout (location = 3) in vec2 vsin_texcoords;


out FS_in
{
    vec3 fragpos;
} vsout;



uniform int un_light_id;
uniform uint un_cascade;

const float[2] CLIP_W = float[2]( 0.5*96.0, 128.0 );


void main()
{
    float xscale = (mat3(IDK_SSBO_Terrain.transform) * vec3(1.0)).x;
    float yscale = IDK_SSBO_Terrain.scale.y;

    float max_xscale = xscale;
    float ratio = pow(2.5, float(4-1)) / xscale;

    if (gl_DrawID == 1)
    {
        xscale = pow(2.5, float(gl_InstanceID)) / ratio;
    }

    else
    {
        xscale = 1.1 / ratio;
    }


    IDK_Camera camera = IDK_UBO_cameras[0];

    float spacing = xscale / CLIP_W[gl_DrawID];
    vec2  cam_xz  = round(camera.position.xz / spacing) * spacing;

    mat4 model = IDK_SSBO_Terrain.transform;
         model[3].xz  = round(model[3].xz / spacing) * spacing;
         model[3].xz += cam_xz;

    vec2 min_xz = max_xscale * vec2(-0.5);
    vec2 max_xz = max_xscale * vec2(+0.5);
    vec2 pos_xz = xscale * vsin_pos.xz + cam_xz;

    vec2 texcoord = (pos_xz - min_xz) / (max_xz - min_xz);
         texcoord = clamp(texcoord, 0.0, 1.0);

    vsout.fragpos    = xscale * vec3(vsin_pos.x, 0.0, vsin_pos.z);
    vsout.fragpos.y += textureLod(IDK_SSBO_Terrain.height, texcoord, 0.0).r;
    vsout.fragpos   += model[3].xyz;


    IDK_Dirlight light = IDK_UBO_dirlights[un_light_id];
    mat4 transform = (un_cascade <= 3) ? light.transforms[un_cascade] : light.transform;

    gl_Position = transform * vec4(vsout.fragpos, 1.0);

}
