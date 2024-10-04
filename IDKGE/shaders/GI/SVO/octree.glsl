#include "../../include/storage.glsl"
#include "../../include/bindings.glsl"
#include "../../include/util.glsl"


struct IDK_OctNode
{
    int children;
    int leaf;
    int pad0;
    int pad1;
};

struct IDK_OctLeaf
{
    // 8x8x8 block covers 1x1x1 in-game units.
    // --> each element is 1/8 in-game units wide.
    uvec4 data[8][8][8];
};

struct IDK_Octree
{
    vec4 span;
    mat4 transform;

    IDK_OctNode nodes [2<<10];
    IDK_OctLeaf leaves[2<<10];
};


layout (std430, binding=IDK_BINDING_SSBO_SVOctree) buffer IDK_SSBO_Octree
{
    IDK_Octree IDK_SSBO_octree;
};


mat4 SVO_GetTransform()
{
    return IDK_SSBO_octree.transform;
}

float SVO_GetRootSpan()
{
    return IDK_SSBO_octree.span[0];
}


uvec3 SVO_WorldToGrid( vec3 worldpos )
{
    worldpos -= SVO_GetTransform()[3].xyz;
    return uvec3(worldpos);
}


IDK_OctNode SVO_GetRoot()
{
    IDK_OctNode root = {0, -1};
    return root;
}


IDK_OctNode SVO_GetChild( IDK_OctNode node, int octant )
{
    int idx = node.children + octant;
    return IDK_SSBO_octree.nodes[idx];
}


bool SVO_IsLeaf( IDK_OctNode node )
{
    return (node.leaf >= 0);
}


bool SVO_HasChildren( IDK_OctNode node )
{
    return (node.children >= 0);
}


void SVO_GetLeaf( int id, out uvec4[8][8][8] data )
{
    data = IDK_SSBO_octree.leaves[id].data;
}




int SVO_GetOctant( vec3 pos, vec3 center )
{
    int octant = 0;

    if (pos.x < center.x) octant |= 1;
    if (pos.y < center.y) octant |= 2;
    if (pos.z < center.z) octant |= 4;

    return octant;
}


vec3 SVO_ShiftCenter( int octant, vec3 center, float span )
{
    vec3 offset;

    offset.x = (octant & 1) == 0 ? span/4.0 : -span/4.0;
    offset.y = (octant & 2) == 0 ? span/4.0 : -span/4.0;
    offset.z = (octant & 4) == 0 ? span/4.0 : -span/4.0;

    return center + offset;
}


int SVO_AllocateLeaf( int node )
{
    return 0;
}


IDK_OctNode SVO_GetNode( vec3 worldpos )
{
    IDK_OctNode node;
    int   octant;
    float span;
    vec3  center;

    node   = SVO_GetRoot();
    span   = SVO_GetRootSpan();
    center = vec3(SVO_GetTransform()[3]);

    while (SVO_HasChildren(node) == true)
    {
        octant = SVO_GetOctant(worldpos, center);
        node   = SVO_GetChild(node, octant);
        center = SVO_ShiftCenter(octant, center, span);
        span   = 0.5 * span;
    }

    // SVO_GetLeaf(node.leaf, data);

    return node;
}

