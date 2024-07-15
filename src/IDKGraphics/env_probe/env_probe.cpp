#include "env_probe.hpp"



int
idk::ProbeHierarchy::get_octant( const glm::vec3 &center, const glm::vec3 &pos )
{
    int octant = 0;

    if (pos.x < center.x) octant |= 1;
    if (pos.y < center.y) octant |= 2;
    if (pos.z < center.z) octant |= 4;

    return octant;
}


glm::vec3
idk::ProbeHierarchy::shift_center( const glm::vec3 &center, int octant )
{
    glm::vec3 pos = center;
    return pos;
}


void
idk::ProbeHierarchy::create_node( int parent, int octant )
{
    m_nodes.push_back(Node());
    m_nodes[parent].children[octant] = m_nodes.size() - 1;
}


void
idk::ProbeHierarchy::create_probe( int parent )
{
    m_probes.push_back(Probe());
    m_nodes[parent].probe = m_probes.size() - 1;
}




idk::ProbeHierarchy::ProbeHierarchy()
:   m_nodes  (1),
    m_probes (MAX_PROBES)
{

}


void
idk::ProbeHierarchy::insert( const glm::vec3 &position, const glm::vec3 &center,
                             int node, float span )
{
    int  octant     = get_octant(center, position);
    auto new_center = shift_center(center, octant);

    if (span <= MIN_SPAN)
    {
        if (m_nodes[node].probe == -1)
        {
            create_probe(node);
        }

        return;
    }


    if (m_nodes[node].children[octant] == -1)
    {
        create_node(node, octant);
    }

    insert(position, new_center, m_nodes[node].children[octant], span/2.0f);

}

