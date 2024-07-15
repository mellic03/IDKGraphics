#pragma once

#include <libidk/idk_glm.hpp>

#include <cstddef>
#include <cstdint>
#include <vector>


namespace idk
{
    class ProbeHierarchy;
}



class idk::ProbeHierarchy
{
public:
    static constexpr int   MAX_PROBES = 512;
    static constexpr float ROOT_SPAN  = 128.0f;
    static constexpr float MIN_SPAN   = 0.5f;

private:

    struct Node
    {
        int probe = -1;
        int children[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };
    };

    struct Probe
    {
        uint32_t  texture;
        uint64_t  handle;
        glm::vec3 position;
    };


    std::vector<Node>  m_nodes;
    std::vector<Probe> m_probes;

    static int         get_octant   ( const glm::vec3 &center, const glm::vec3 &pos );
    static glm::vec3   shift_center ( const glm::vec3 &center, int octant );
    void               create_node  ( int parent, int octant );
    void               create_probe ( int parent );


public:

    ProbeHierarchy();

    void insert( const glm::vec3&, const glm::vec3 &center=glm::vec3(0.0f),
                 int node=0, float span=ROOT_SPAN );

    const std::vector<Node>  &getNodes()  const { return m_nodes;  };
    const std::vector<Probe> &getProbes() const { return m_probes; };

};
