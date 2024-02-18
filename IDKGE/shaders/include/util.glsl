

vec3 IDK_WorldToUV( vec3 world_position, mat4 PV )
{
    vec4 proj = PV * vec4(world_position, 1.0);
    proj.xy = (proj.xy / proj.w) * 0.5 + 0.5;

    return proj.xyz;
}

ivec2 IDK_WorldToTexel( vec3 world_position, mat4 PV, vec2 image_size )
{
    vec2 uv = IDK_WorldToUV(world_position, PV).xy;
    return ivec2(uv * image_size);
}


vec3 IDK_UVToWorld( vec2 uv, mat4 view_matrix )
{
    return vec3(0.0);
}

vec3 IDK_TexelToWorld( ivec2 texel, mat4 view_matrix )
{

    return vec3(0.0);
}


float IDK_DepthAtTexel( ivec2 texel, mat4 PV, image2D image )
{
    float frag_depth = (PV * imageLoad(image, texel)).z;
    return frag_depth;
}


// float IDK_DepthAtUV( vec2 uv, mat4 PV, image2D image )
// {
//     float frag_depth = (PV * texture(un_texture_1, uv)).z;
//     float ray_depth = projected.z;
// }
