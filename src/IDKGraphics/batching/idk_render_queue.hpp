#pragma once

#include <libidk/idk_glm.hpp>
#include "../storage/idk_ssbo_indirect.hpp"
#include "../batching/idk_model_allocator.hpp"
#include "../camera/idk_camera.hpp"

#include <vector>
#include <unordered_map>



namespace idk
{
    class RenderQueue;
    class ModelAllocator;
};




class idk::RenderQueue
{
private:
    std::unordered_map<int, std::vector<glm::mat4>> m_drawlist;

    glDrawCmd genDrawCommand( int, idk::MeshDescriptor & );
    glDrawCmd genDrawCommand( int, idk::MeshDescriptor &, size_t );


public:
    std::string name;

    void enque( int, const glm::mat4 & );
    void clear();

    const std::vector<glDrawCmd> &genDrawCommands( idk::DrawIndirectData &, idk::ModelAllocator & );
    const std::vector<glDrawCmd> &genDrawCommands( idk::DrawIndirectData &, idk::ModelAllocator &, idk::Camera & );

};

