#version 460 core
#extension GL_GOOGLE_include_directive: require
#include "octree.glsl"


out vec4 fsout_color;
out vec2 fsin_texcoords;

uniform sampler2D un_fragcolor;
uniform sampler2D un_fragdepth;



void SVO_WriteLeaf( int leaf, uvec3 grid, vec3 irradiance )
{
    uvec3 u = uvec3(255.0*irradiance);

    uint  x = grid.x;
    uint  y = grid.y;
    uint  z = grid.z;

    atomicAdd(IDK_SSBO_octree.leaves[leaf].data[z][y][x].r, u.r);
    atomicAdd(IDK_SSBO_octree.leaves[leaf].data[z][y][x].g, u.g);
    atomicAdd(IDK_SSBO_octree.leaves[leaf].data[z][y][x].b, u.b);
    atomicAdd(IDK_SSBO_octree.leaves[leaf].data[z][y][x].a, 1);
}


void SVO_AddIrradiance( vec3 worldpos, vec3 irradiance )
{
    uvec3       grid = SVO_WorldToGrid(worldpos);
    IDK_OctNode node = SVO_GetNode(worldpos);

    if (SVO_IsLeaf(node) == false)
    {
        // Something has gone very wrong
    }

    else
    {
        SVO_WriteLeaf(node.leaf, grid, irradiance);
    }
}


void main()
{
    IDK_Camera cam = IDK_GetCamera();

    vec3 worldpos = IDK_WorldFromDepth(un_fragdepth, fsin_texcoords, cam.P, cam.V);
    vec3 color    = texture(un_fragcolor, fsin_texcoords).rgb;


    

}


