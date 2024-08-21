#version 460 core

#extension GL_GOOGLE_include_directive: require
#extension GL_ARB_bindless_texture: require

#include "../include/storage.glsl"
#include "./terrain.glsl"

layout(triangles, fractional_even_spacing, ccw) in;


in TESE_in
{
    vec3 fragpos;
    vec3 normal;
    vec3 tangent;
    vec2 texcoord;
    float tesslevel;
    mat3 TBN;

} tsin[];


out FS_in
{
    vec3 fragpos;
    vec3 normal;
    vec3 tangent;
    vec2 texcoord;
    float tesslevel;
    float height;
    mat3 TBN;

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


uniform int un_light_id;
uniform uint un_cascade = 0;

void main()
{
    tsout.texcoord = interpolate2D(tsin[0].texcoord, tsin[1].texcoord, tsin[2].texcoord);
    tsout.tangent  = interpolate3D(tsin[0].tangent,  tsin[1].tangent,  tsin[2].tangent);
    tsout.fragpos  = interpolate3D(tsin[0].fragpos,  tsin[1].fragpos,  tsin[2].fragpos);
    tsout.normal   = interpolate3D(tsin[0].normal,   tsin[1].normal,   tsin[2].normal);
    tsout.TBN      = tsin[0].TBN;

    mat4 model = IDK_SSBO_Terrain.transform;

    vec3 normal = texture(IDK_SSBO_Terrain.nmap, tsout.texcoord).rgb * 2.0 - 1.0;

    float scale = IDK_SSBO_Terrain.texscale[0][0];
    float disp  = texture(IDK_SSBO_Terrain.disp, vec3(scale*tsout.texcoord, 0)).r;
          disp *= IDK_SSBO_Terrain.scale[2];

    float xscale = (mat3(IDK_SSBO_Terrain.transform) * vec3(1.0)).x;
    float yscale = IDK_SSBO_Terrain.scale.y;

    tsout.height     = textureLod(IDK_SSBO_Terrain.height, tsout.texcoord, 0.0).r;
    tsout.fragpos.y  = model[3].y + tsout.height;
    tsout.height /= (yscale);
    tsout.normal     = normalize(normal);
    // tsout.fragpos   += tsout.normal * disp;

    tsout.tesslevel = tsin[0].tesslevel;

    IDK_Camera camera = IDK_UBO_cameras[0];
    IDK_Dirlight light = IDK_UBO_dirlights[un_light_id];

    mat4 A = camera.P * camera.V;
    mat4 B = light.transforms[un_cascade];

    mat4 T = (un_light_id == -1) ? A : B;

    gl_Position = T * vec4(tsout.fragpos, 1.0);

} 

