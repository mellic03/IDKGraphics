#include "../../../IDKGE/shaders/include/storage.glsl"
#include "../../../IDKGE/shaders/include/bindings.glsl"
#include "../../../IDKGE/shaders/include/util.glsl"



struct IDK_IrradianceCache
{
    mat4  transform;
    uvec4 data[128][128][128];
};


layout (std430, binding=IDK_BINDING_SSBO_Irradiance) buffer IDK_SSBO_Irradiance
{
    IDK_IrradianceCache IDK_SSBO_IrradianceCache;
};


vec3 IrradianceWorldToLocal( vec3 worldpos )
{
    return (IDK_SSBO_IrradianceCache.transform * vec4(worldpos, 1.0)).xyz;
}


uvec3 IrradianceWorldToGrid( vec3 worldpos )
{
    return uvec3(IrradianceWorldToLocal(worldpos));
}


bool IrradianceInBounds( vec3 worldpos )
{
    uvec3 grid = IrradianceWorldToGrid(worldpos);
    return (grid == clamp(grid, uvec3(0), uvec3(128)));
}


vec3 IrradianceSampleGrid( uvec3 grid )
{
    uvec3 irradiance = IDK_SSBO_IrradianceCache.data[grid.z][grid.y][grid.x].rgb;
    return vec3(irradiance) / 255.0;
}


vec3 IrradianceSampleWorld( vec3 worldpos )
{
    uvec3 grid = IrradianceWorldToGrid(worldpos);
    return IrradianceSampleGrid(grid);
}


void addIrradiance( vec3 worldpos, vec3 color )
{
    vec3 local = (IDK_SSBO_IrradianceCache.transform * vec4(worldpos, 1.0)).xyz;

    int x = clamp(int(local.x), 0, 128-1);
    int y = clamp(int(local.y), 0, 128-1);
    int z = clamp(int(local.z), 0, 128-1);

    atomicAdd(IDK_SSBO_IrradianceCache.data[z][y][x].r, uint(255.0 * color.r));
    atomicAdd(IDK_SSBO_IrradianceCache.data[z][y][x].g, uint(255.0 * color.g));
    atomicAdd(IDK_SSBO_IrradianceCache.data[z][y][x].b, uint(255.0 * color.b));
    atomicAdd(IDK_SSBO_IrradianceCache.data[z][y][x].a, 1);
}

