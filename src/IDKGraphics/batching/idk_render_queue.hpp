#pragma once

#include <libidk/idk_glm.hpp>

#include "../storage/buffers.hpp"
#include "../batching/idk_model_allocator.hpp"
#include "../camera/idk_camera.hpp"

#include <vector>
#include <unordered_map>


namespace idk
{
    struct Transform;

    struct RenderQueueConfig;
    class  RenderQueue;
    class  ModelAllocator;
};


struct idk::RenderQueueConfig
{
    bool cull_face = true;
};



class idk::RenderQueue
{
private:
    std::unordered_map<int, std::vector<std::pair<glm::mat4, glm::mat4>>> m_drawlist;

    size_t    m_drawcmd_offset = 0;
    size_t    m_num_drawcmd    = 0;

    glDrawCmd genDrawCommand( int, idk::MeshDescriptor & );
    glDrawCmd genDrawCommand( int, idk::MeshDescriptor &, size_t );


public:
    idk::RenderQueueConfig config;
    std::string            name;
    int                    ID = -1;

    void enque( int, const glm::mat4& );
    void enque( int, const glm::mat4&, const glm::mat4& );
    void enque( int, const idk::Transform&, const IDK_Camera&, ModelAllocator& );
    void clear();
    bool empty() { return m_drawlist.empty(); };

    size_t getDrawCommandOffset() { return m_drawcmd_offset;  };
    size_t numDrawCommands()      { return m_num_drawcmd;     };
    bool   empty2()               { return m_num_drawcmd > 0; };

    std::vector<std::pair<glm::mat4, glm::mat4>> &getDrawList( int model)
    {
        return m_drawlist[model];
    }

    void genDrawCommands( ModelAllocator&, size_t&, size_t&, size_t&, SSBO_Buffer&,
                          std::vector<idk::glDrawCmd>& );

};

