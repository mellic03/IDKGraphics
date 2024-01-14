#pragma once

#include <libidk/idk_allocator.hpp>
#include <libidk/GL/common.hpp>
#include <libidk/GL/idk_glXXBO.hpp>

#include <libidk/idk_singleton.hpp>

#include "../model/idk_model.hpp"
#include "idk_model_handle.hpp"
#include "idk_batch_allocator.hpp"



namespace idk { class ModelAllocator; };


class idk::ModelAllocator
{
private:

    static constexpr size_t     BATCH_NBYTES = 128 * idk::MEGA;

    std::vector<BatchAllocator> m_batches;

    Allocator<idk::glTexture>   m_textures;
    Allocator<GLuint64>         m_texture_handles;
    Allocator<idk::Material>    m_materials;
    Allocator<ModelHandle>      m_model_handles;


    int _createModel();
    int _createMaterial();



public:
        ModelAllocator();


    idk::ModelHandle loadModel( const std::string &filepath );


    const idk::ModelHandle &getModelHandle( int id );
};

// namespace idk::BatchSystem
// {
//     size_t init( size_t bytes_per_batch );

//     int load( uint32_t vertexformat, size_t v_nbytes, float *vertices,
//               size_t i_nbytes, uint32_t *indices );

//     const idk::ModelHandle &getModelHandle( int id );

// };

