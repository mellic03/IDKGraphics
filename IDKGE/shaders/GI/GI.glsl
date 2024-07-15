#include "../include/bindings.glsl"

uniform ivec3 un_probe_grid_size; // = ivec3(8, 3, 8);
uniform  vec3 un_probe_cell_size; // = vec3(2.0, 3.0, 2.0);

uniform samplerCubeArray IDK_UN_PROBE_ARRAY;


#define IDK_PROBE_CELL_SPACING un_probe_cell_size

#define IDK_PROBE_GRID_X       un_probe_grid_size.x
#define IDK_PROBE_GRID_Y       un_probe_grid_size.y
#define IDK_PROBE_GRID_Z       un_probe_grid_size.z
#define IDK_PROBE_GRID_NPROBES (IDK_PROBE_GRID_X * IDK_PROBE_GRID_Y * IDK_PROBE_GRID_Z)

#define IDK_PROBE_GRID_SIZE  un_probe_grid_size
#define IDK_PROBE_GRID_HSIZE (un_probe_grid_size / 2)




int IDK_GI_WorldToLayer( vec3 world )
{
    world /= IDK_PROBE_CELL_SPACING;
    world += vec3(IDK_PROBE_GRID_HSIZE);

    const int w = IDK_PROBE_GRID_X;
    const int h = IDK_PROBE_GRID_Y;
    const int d = IDK_PROBE_GRID_Z;

    ivec3 cell = ivec3(world);
          cell = clamp(cell, ivec3(0), IDK_PROBE_GRID_SIZE);

    int layer = w*h*cell.z + w*cell.y + cell.x;

    return layer;
}



vec3 IDK_GI_SampleGrid( ivec3 cell, vec3 N )
{
    const int w = IDK_PROBE_GRID_X;
    const int h = IDK_PROBE_GRID_Y;
    const int d = IDK_PROBE_GRID_Z;

    int   layer = w*h*cell.z + w*cell.y + cell.x;

    return textureLod(IDK_UN_PROBE_ARRAY, vec4(N, layer), 0).rgb;
}


float linearDepth( float d )
{
    const float zNear = 0.001;
    const float zFar = 32.0;
    return zNear * zFar / (zFar + d * (zNear - zFar));
}



int IDK_GI_WorldToLayer_Nearest( vec3 world )
{
    world /= IDK_PROBE_CELL_SPACING;
    world += vec3(IDK_PROBE_GRID_HSIZE);

    const int w = IDK_PROBE_GRID_X;
    const int h = IDK_PROBE_GRID_Y;
    const int d = IDK_PROBE_GRID_Z;

    ivec3 cell = ivec3(world + 0.5);
          cell = clamp(cell, ivec3(0), IDK_PROBE_GRID_SIZE);

    int layer = w*h*cell.z + w*cell.y + cell.x;

    return layer;
}