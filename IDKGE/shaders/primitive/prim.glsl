#include "../include/bindings.glsl"

#ifndef IDK_PRIMITIVE
#define IDK_PRIMITIVE

struct IDK_PrimSurface
{
    vec4  albedo;
    float roughness;
    float metallic;
    float emission;
    float padding;
};

struct IDK_Prim
{
    vec4  albedo;
    float roughness;
    float metallic;
    float emission;
    float padding;

    mat4 T;
    mat4 prev_T;

};

layout (std140, binding=IDK_BINDING_UBO_Primitives) uniform IDK_UBO_Primitives
{
    IDK_Prim UBO_Primitives[2][2048];
};




#endif