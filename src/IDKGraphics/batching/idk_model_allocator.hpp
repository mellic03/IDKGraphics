#pragma once

#include <libidk/idk_allocator.hpp>

#include <libidk/GL/common.hpp>
#include <libidk/GL/idk_glXXBO.hpp>
#include <libidk/GL/idk_glTexture.hpp>
#include <libidk/GL/idk_glSSBO.hpp>

#include "idk_model.hpp"
#include "idk_mesh_allocator.hpp"

#include <unordered_set>
#include <unordered_map>


namespace idk
{
    class  ModelAllocator;
    struct SSBOModelData;
};


#define IDK_MODELDATA_MAX_MATERIALS  256
#define IDK_MODELDATA_MAX_TRANSFORMS 256

struct idk::SSBOModelData
{
    GLuint64    materials   [IDK_MODELDATA_MAX_MATERIALS][8];
    glm::mat4   transforms  [IDK_MODELDATA_MAX_TRANSFORMS];
};





class idk::ModelAllocator
{
private:
    static constexpr uint32_t MAX_DRAW_COMMANDS = 128;

    uint32_t m_default_textures[5];
    uint64_t m_default_handles[5];

    idk::vector<idk::MeshAllocator>         m_mesh_allocators;

    std::unordered_map<std::string, GLuint> m_texture_cache;

    std::unordered_set<std::string>         m_loaded_models;
    std::unordered_map<std::string, int>    m_loaded_model_IDs;

    idk::glTextureConfig                    m_albedo_config;
    idk::glTextureConfig                    m_lightmap_config;

    // idk::Allocator<idk::MaterialDescriptor> m_materials;
    idk::Allocator<idk::MeshDescriptor>     m_meshes;
    idk::Allocator<idk::ModelDescriptor>    m_models;


    // SSBO
    // -----------------------------------------------------------------------------------------
    using SSBO_type = idk::glTemplatedBufferObject<GL_SHADER_STORAGE_BUFFER, idk::SSBOModelData>;

    idk::SSBOModelData                      m_ModelData;
    SSBO_type                               m_ModelData_SSBO;
    // -----------------------------------------------------------------------------------------

    using drawlist_type = std::vector<std::pair<int, glm::mat4>>;
    drawlist_type m_drawlist;


    void loadMaterial( uint32_t bitmask, std::string textures[draw_buffer::TEXTURES_PER_MATERIAL],
                       idk::MeshDescriptor& );


public:
        ModelAllocator();

    // IDK_ALLOCATOR_ACCESS(Texture,  idk::TextureDescriptor,  m_textures);
    // IDK_ALLOCATOR_ACCESS(Material, idk::MaterialDescriptor, m_materials);
    IDK_ALLOCATOR_ACCESS(Mesh,     idk::MeshDescriptor,     m_meshes);
    IDK_ALLOCATOR_ACCESS(Model,    idk::ModelDescriptor,    m_models);

    int loadTexture( const std::string &filepath, uint32_t&, uint64_t&, const idk::glTextureConfig & );
    int loadModel( const std::string &filepath, uint32_t format=0 );

    void addUserMaterials( int model, const std::vector<uint32_t> &textures );


    void clear();

    void getVertices( int model_id, size_t &num_vertices, std::unique_ptr<idk::Vertex_P_N_T_UV[]> &vertices );
    void getIndices( int model_id, size_t &num_indices, std::unique_ptr<uint32_t[]> &indices );

    GLuint getVAO( uint32_t format = 0 )    { return m_mesh_allocators[0].VAO;   };

};

