#include "idk_model_allocator.hpp"

#include <libidk/idk_texturegen.hpp>
#include <libidk/GL/idk_gltools.hpp>

#include <IDKThreading/IDKThreading.hpp>

#include <cstring>
#include <filesystem>

namespace fs = std::filesystem;


idk::ModelAllocator::ModelAllocator()
{
    m_mesh_allocators.resize(1);

    m_mesh_allocators[0] = idk::MeshAllocator(
        128 * idk::MEGA, idk::ModelVertexFormat_POS_NORM_TAN_UV
    );

    // m_mesh_allocators[1] = idk::MeshAllocator(
    //     64 * idk::MEGA, idk::ModelVertexFormat_POS_NORM_TAN_UV_SKINNED
    // );

    m_albedo_config = {
        .internalformat = GL_SRGB8_ALPHA8,
        .format         = GL_RGBA,
        .minfilter      = GL_LINEAR_MIPMAP_LINEAR,
        .magfilter      = GL_LINEAR,
        .wrap_s         = GL_REPEAT,
        .wrap_t         = GL_REPEAT,
        .datatype       = GL_UNSIGNED_BYTE,
        .anisotropic    = GL_TRUE,
        .genmipmap      = GL_TRUE,
        .bindless       = GL_TRUE
    };

    m_lightmap_config = {
        .internalformat = GL_RGB8,
        .format         = GL_RGB,
        .minfilter      = GL_LINEAR_MIPMAP_LINEAR,
        .magfilter      = GL_LINEAR,
        .wrap_s         = GL_REPEAT,
        .wrap_t         = GL_REPEAT,
        .datatype       = GL_UNSIGNED_BYTE,
        .anisotropic    = GL_TRUE,
        .genmipmap      = GL_TRUE,
        .bindless       = GL_TRUE
    };


    constexpr size_t IMG_W = 32;

    auto data_albedo = texturegen::genRGBA<uint8_t>(IMG_W, IMG_W, 200, 200, 200, 255 );
    auto data_normal = texturegen::genRGB<uint8_t>(IMG_W, IMG_W, 125, 125, 255);
    auto data_ao_r_m = texturegen::genRGB<uint8_t>(IMG_W, IMG_W, 255, 180, 1);
    auto data_emissv = texturegen::genRGB<uint8_t>(IMG_W, IMG_W, 0, 0, 0);

    GLuint textures[5];
    GLuint handles[5];

    m_default_textures[0] = gltools::loadTexture2D(IMG_W, IMG_W, data_albedo.get(), m_albedo_config);
    m_default_textures[1] = gltools::loadTexture2D(IMG_W, IMG_W, data_normal.get(), m_lightmap_config);
    m_default_textures[2] = gltools::loadTexture2D(IMG_W, IMG_W, data_ao_r_m.get(), m_lightmap_config);
    m_default_textures[3] = gltools::loadTexture2D(IMG_W, IMG_W, data_emissv.get(), m_lightmap_config);
    m_default_textures[4] = m_default_textures[3];

    for (int i=0; i<5; i++)
    {
        m_default_handles[i] = gl::getTextureHandleARB(m_default_textures[i]);
    }

    m_error_model = loadModel("IDKGE/resources/error.idkvi");
}


idk::ModelDescriptor&
idk::ModelAllocator::getModel( int model )
{
    auto &desc = m_models.get(model);

    if (desc.proxy != -1)
    {
        return m_models.get(desc.proxy);
    }

    return desc;
}


idk::ModelDescriptor&
idk::ModelAllocator::getModelLOD( int model, int level )
{
    int lod = getModel(model).LOD[level];
    return getModel(lod);
}




int
idk::ModelAllocator::loadTexture( const std::string &filepath, uint32_t &texture, uint64_t &handle,
                                  const glTextureConfig &config )
{
    if (m_texture_cache.contains(filepath))
    {
        texture = m_texture_cache[filepath];
        handle  = gl::getTextureHandleARB(texture);
        return 0;
    }

    texture = gltools::loadTexture(filepath, config);
    handle  = gl::getTextureHandleARB(texture);

    m_texture_cache[filepath] = texture;

    return 0;
}



struct TestData
{
    std::string filepath = "";
    void *pixels = nullptr;
    uint32_t w, h;
    uint32_t texture = 0;
};



int
idk::ModelAllocator::loadMaterial( uint32_t bitmask,
                                   std::string textures[draw_buffer::TEXTURES_PER_MATERIAL],
                                   idk::MeshDescriptor &mesh )
{
    int material_id = createMaterial();
    auto &material = getMaterial(material_id);

    for (int i=0; i<5; i++)
    {
        material.textures[i] = m_default_textures[i];
        material.handles[i]  = m_default_handles[i];
    }

    std::vector<TestData> *data = new std::vector<TestData>(draw_buffer::TEXTURES_PER_MATERIAL);

    for (int i=0; i<draw_buffer::TEXTURES_PER_MATERIAL; i++)
    {
        (*data)[i].filepath = textures[i];
    }

    idk::ThreadPool::createTask(
        [this, bitmask, material_id, data]()
        {
            for (int i=0; i<draw_buffer::TEXTURES_PER_MATERIAL; i++)
            {
                if (idk::MeshFile_hasTexture(bitmask, i) == false)
                {
                    continue;
                }

                if (m_texture_cache.contains((*data)[i].filepath))
                {
                    auto &mat = getMaterial(material_id);
                    (*data)[i].texture = m_texture_cache[(*data)[i].filepath];
                    continue;
                }
            
                const auto &config = (i == 0) ? m_albedo_config : m_lightmap_config;

                (*data)[i].pixels = gltools::loadPixels(
                    (*data)[i].filepath,
                    &((*data)[i].w),
                    &((*data)[i].h),
                    (i == 0)
                );
            }
        },

        [this, bitmask, material_id, data]()
        {
            auto &mat = getMaterial(material_id);

            for (int i=0; i<draw_buffer::TEXTURES_PER_MATERIAL; i++)
            {
                if (idk::MeshFile_hasTexture(bitmask, i) == false)
                {
                    continue;
                }

                const auto &config = (i == 0) ? m_albedo_config : m_lightmap_config;

                if ((*data)[i].texture > 0)
                {
                    mat.textures[i] = (*data)[i].texture;

                }

                else
                {
                    mat.textures[i] = gltools::loadTexture2D(
                        (*data)[i].w,
                        (*data)[i].h,
                        (*data)[i].pixels,
                        config
                    );
                }

                mat.handles[i] = gl::getTextureHandleARB(mat.textures[i]);
            }


            for (int i=0; i<draw_buffer::TEXTURES_PER_MATERIAL; i++)
            {
                if ((*data)[i].pixels)
                {
                    std::free((*data)[i].pixels);
                }
            }

            delete data;
        }
    );

    return material_id;
}


int
idk::ModelAllocator::loadModel( const std::string &filepath, uint32_t format )
{
    if (fs::exists(filepath) == false)
    {
        LOG_ERROR() << "File does not exist: \"" << filepath << "\"";

        return m_error_model;

        // std::cout << "Cannot open \"" << filepath << "\"\n";
        // IDK_ASSERT("File does not exist", fs::exists(filepath));
    }

    if (m_loaded_model_IDs.contains(filepath))
    {
        std::cout << "Model already cached: \"" << filepath << "\"\n";
        return m_loaded_model_IDs[filepath];
    }

    // std::cout << "Loading model from file: \"" << filepath << "\"\n";

    idk::ModelFileHeader header;
    std::vector<idk::MeshFileHeader> meshes;
    std::vector<void *> vertices;
    std::vector<void *> indices;

    idk::ModelFile_read(filepath, header, meshes, vertices, indices);
    idk::ModelDescriptor model_desc;

    for (uint32_t i=0; i<header.num_meshes; i++)
    {
        auto &mesh = meshes[i];

        idk::MeshDescriptor desc = m_mesh_allocators[format].loadMesh(
            mesh.num_verts * idk::VertexFormat_sizeof(header.vertexformat),
            mesh.num_indices * sizeof(uint32_t),
            vertices[i],
            indices[i]
        );

        desc.material = loadMaterial(mesh.bitmask, mesh.textures, desc);
        desc.bounding_radius = meshes[i].bounding_radius;

        model_desc.meshes.push_back(desc);
    }

    int model_id = m_models.create(model_desc);

    for (auto &mesh: meshes)
    {
        auto &rad = getModel(model_id).bounding_radius;
              rad = glm::max(rad, mesh.bounding_radius);
    }

    for (int i=0; i<4; i++)
    {
        getModel(model_id).LOD[i] = model_id;
    }

    m_loaded_model_IDs[filepath] = model_id;

    for (auto &data: vertices) { std::free(data); };
    for (auto &data: indices)  { std::free(data); };

    return model_id;
}


int
idk::ModelAllocator::loadModelLOD( int model, int level, const std::string &filepath, uint32_t format )
{
    int model_lod = loadModel(filepath, format);
    getModel(model).LOD[level] = model_lod;

    return model_lod;
}


void
idk::ModelAllocator::addUserMaterials( int model, const std::vector<std::string> &textures,
                                       const idk::glTextureConfig &config )
{
    // for (auto &mesh: getModel(model).meshes)
    // {
    //     if (mesh.textures.size() < textures.size())
    //     {
    //         mesh.textures.resize(textures.size());
    //         mesh.handles.resize(textures.size());
    //     }

    //     for (size_t i=0; i<textures.size(); i++)
    //     {
    //         loadTexture(textures[i], mesh.textures[i], mesh.handles[i], config);
    //     }
    // }
}



void
idk::ModelAllocator::clear()
{
    m_meshes.clear();
    m_models.clear();

    for (auto &allocator: m_mesh_allocators)
    {
        allocator.clear();
    }

}



void
idk::ModelAllocator::getVertices( int model_id, size_t &num_vertices, std::unique_ptr<idk::Vertex_P_N_T_UV[]> &vertices )
{
    num_vertices = 0;

    for (MeshDescriptor &mesh: getModel(model_id).meshes)
    {
        num_vertices += mesh.numVertices;
    }

    vertices = std::make_unique<Vertex_P_N_T_UV[]>(num_vertices);
    Vertex_P_N_T_UV *buffer = (Vertex_P_N_T_UV *)gl::mapNamedBuffer(m_mesh_allocators[0].VBO, GL_READ_ONLY);

    size_t offset = 0;
    for (MeshDescriptor &mesh: getModel(model_id).meshes)
    {
        std::memcpy(
            vertices.get() + offset,
            buffer + mesh.baseVertex,
            mesh.numVertices * sizeof(Vertex_P_N_T_UV)
        );

        offset += mesh.numVertices;
    }

    gl::unmapNamedBuffer(m_mesh_allocators[0].VBO);
}


void
idk::ModelAllocator::getIndices( int model_id, size_t &num_indices, std::unique_ptr<uint32_t[]> &indices )
{
    num_indices = 0;

    for (MeshDescriptor &mesh: getModel(model_id).meshes)
    {
        num_indices += mesh.numIndices;
    }

    indices = std::make_unique<uint32_t[]>(num_indices);
    uint32_t *buffer = (uint32_t *)gl::mapNamedBuffer(m_mesh_allocators[0].IBO, GL_READ_ONLY);

    size_t offset = 0;
    for (MeshDescriptor &mesh: getModel(model_id).meshes)
    {
        std::memcpy(
            indices.get() + offset,
            buffer + mesh.firstIndex,
            mesh.numIndices * sizeof(uint32_t)
        );

        offset += mesh.numIndices;
    }

    gl::unmapNamedBuffer(m_mesh_allocators[0].IBO);
}


