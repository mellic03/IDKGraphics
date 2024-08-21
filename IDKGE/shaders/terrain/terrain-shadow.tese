#version 460 core

#extension GL_GOOGLE_include_directive: require
#extension GL_ARB_bindless_texture: require

#include "../include/storage.glsl"
#include "./terrain.glsl"

layout(triangles, equal_spacing, ccw) in;

uniform uint un_light_id;
uniform uint un_cascade;

in TESE_in
{
    vec3 fragpos;
    vec3 normal;
    vec3 tangent;
    vec2 texcoord;

} tsin[];


out FS_in
{
    vec3 fragpos;
    vec3 normal;
    vec3 tangent;
    vec2 texcoord;

} tsout;



vec2 interpolate2D( vec2 v0, vec2 v1, vec2 v2 )
{
    return vec2(gl_TessCoord.x) * v0 + vec2(gl_TessCoord.y) * v1 + vec2(gl_TessCoord.z) * v2;
}

vec3 interpolate3D( vec3 v0, vec3 v1, vec3 v2 )
{
    return vec3(gl_TessCoord.x) * v0 + vec3(gl_TessCoord.y) * v1 + vec3(gl_TessCoord.z) * v2;
}

vec3 computeNormal( vec3 a, vec3 b, vec3 c )
{
    return normalize(cross(b-a, c-a));
}


void main()
{
    // Interpolate the attributes of the output vertex using the barycentric coordinates
    tsout.texcoord = interpolate2D(tsin[0].texcoord, tsin[1].texcoord, tsin[2].texcoord);
    tsout.normal   = interpolate3D(tsin[0].normal, tsin[1].normal, tsin[2].normal);
    tsout.fragpos  = interpolate3D(tsin[0].fragpos, tsin[1].fragpos, tsin[2].fragpos);

    mat4 model = IDK_SSBO_Terrain.transform;
    tsout.fragpos.y  = model[3].y + textureLod(IDK_SSBO_Terrain.height, tsout.texcoord, 0.0).r;
    tsout.fragpos    = vec3(IDK_SSBO_Terrain.transform * vec4(tsout.fragpos, 1.0));

    IDK_Dirlight light = IDK_UBO_dirlights[un_light_id];
    mat4 transform = light.transforms[un_cascade];

    gl_Position = transform * vec4(tsout.fragpos, 1.0);

} 

