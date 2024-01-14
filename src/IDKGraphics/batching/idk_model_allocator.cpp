#include "idk_model_allocator.hpp"
#include <libidk/idk_idkvi_file.hpp>


idk::ModelAllocator::ModelAllocator()
{
    m_batches.push_back(
        BatchAllocator(
            BATCH_NBYTES, idk::VertexFormat::VERTEX_POSITION3F_NORMAL3F_TANGENT3F_UV2F
        )
    );

    // for (uint32_t i=0; i<idk::VERTEX_NUM_FORMATS; i++)
    // {
    //     m_batches.push_back(BatchAllocator(BATCH_NBYTES, i));
    // }
}


idk::ModelHandle
idk::ModelAllocator::loadModel( const std::string &filepath )
{
    idkvi_file file = idk::idkvi_load(filepath.c_str());

    auto &batch = m_batches[0];

    idk::ModelHandle handle = batch.load(
        file.vertices.nbytes(),
        file.indices.nbytes(),
        file.vertices.data(),
        file.indices.data()
    );


    std::cout
        << file.vertexformat << "\n"
        << file.indices.size() << "\n"
        << file.vertices.size() << "\n";


    // for (auto &mesh: file.meshes)
    // {
    //     for (auto &texture: mesh.material.textures)
    //     {
    //         std::cout << texture << "\n";
    //     }
    // }

    std::cout << "Model loaded\n";
    // std::cout << handle.basevertex << ", " << handle.baseindex << "\n";

    return handle;

}




const idk::ModelHandle &
idk::ModelAllocator::getModelHandle( int id )
{
    return m_model_handles.get(id);
}

