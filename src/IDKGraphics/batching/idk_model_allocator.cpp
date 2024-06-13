#include "idk_model_allocator.hpp"

#include <libidk/idk_texturegen.hpp>
#include <libidk/GL/idk_gltools.hpp>

#include <cstring>
#include <filesystem>

namespace fs = std::filesystem;


idk::ModelAllocator::ModelAllocator()
{
    m_mesh_allocators.resize(2);

    m_mesh_allocators[0] = idk::MeshAllocator(
        2048 * idk::MEGA, idk::ModelVertexFormat_POS_NORM_TAN_UV
    );

    m_mesh_allocators[1] = idk::MeshAllocator(
        512 * idk::MEGA, idk::ModelVertexFormat_POS_NORM_TAN_UV_SKINNED
    );

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
        .internalformat = GL_RGBA16,
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


    constexpr size_t IMG_W = 32;

    auto data_albedo = texturegen::genRGBA<uint8_t>(IMG_W, IMG_W, 200, 200, 200, 255 );
    auto data_normal = texturegen::genRGBA<uint8_t>(IMG_W, IMG_W, 125, 125, 255, 0);
    auto data_ao_r_m = texturegen::genRGBA<uint8_t>(IMG_W, IMG_W, 255, 180, 1, 0);
    auto data_emissv = texturegen::genRGBA<uint8_t>(IMG_W, IMG_W, 0, 0, 0, 255);

    GLuint textures[5];

    textures[0] = gltools::loadTexture2D(IMG_W, IMG_W, data_albedo.get(), m_albedo_config);
    textures[1] = gltools::loadTexture2D(IMG_W, IMG_W, data_normal.get(), m_lightmap_config);
    textures[2] = gltools::loadTexture2D(IMG_W, IMG_W, data_ao_r_m.get(), m_lightmap_config);
    textures[3] = gltools::loadTexture2D(IMG_W, IMG_W, data_emissv.get(), m_lightmap_config);
    textures[4] = gltools::loadTexture2D(IMG_W, IMG_W, data_emissv.get(), m_lightmap_config);

    for (int i=0; i<5; i++)
    {
        TextureDescriptor desc = {
            .texture  = textures[i],
            .handle = gl::getTextureHandleARB(textures[i])
        };

        createTexture(desc);
    }


    m_ModelData_SSBO.init(1);

}


int
idk::ModelAllocator::loadTexture( const std::string &filepath, const glTextureConfig &config )
{
    if (m_loaded_textures.contains(filepath))
    {
        return m_loaded_textures[filepath];
    }

    TextureDescriptor desc;
    desc.texture  = gltools::loadTexture(filepath, config);
    desc.handle = gl::getTextureHandleARB(desc.texture);

    int id = createTexture(desc);
    m_loaded_textures[filepath] = id;

    return id;
}


int
idk::ModelAllocator::loadMaterial( uint32_t bitmask, std::string textures[IDK_TEXTURES_PER_MATERIAL] )
{
    idk::MaterialDescriptor desc;
    
    for (int i=0; i<IDK_TEXTURES_PER_MATERIAL; i++)
    {
        desc.textures[i] = i;
    }

    for (int i=0; i<IDK_TEXTURES_PER_MATERIAL; i++)
    {
        if (idk::MeshFile_hasTexture(bitmask, i))
        {
            const auto &config = (i == 0) ? m_albedo_config : m_lightmap_config;
            desc.textures[i] = loadTexture(textures[i], config);
        }
    }

    return createMaterial(desc);
}


int
idk::ModelAllocator::loadModel( const std::string &filepath, uint32_t format )
{
    if (fs::exists(filepath) == false)
    {
        std::cout << "Cannot open \"" << filepath << "\"\n";
        IDK_ASSERT("File does not exist", fs::exists(filepath));
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

        desc.material_id = loadMaterial(mesh.bitmask, mesh.textures);
        desc.bounding_radius = meshes[i].bounding_radius;

        model_desc.mesh_ids.push_back(createMesh(desc));
    }

    int model_id = createModel(model_desc);
    m_loaded_model_IDs[filepath] = model_id;

    for (auto &data: vertices) { std::free(data); };
    for (auto &data: indices)  { std::free(data); };

    return model_id;
}


int
idk::ModelAllocator::createMaterial( int albedo, int normal, int ao_r_m )
{
    int id = createMaterial();

    getMaterial(id).textures[0] = albedo;
    getMaterial(id).textures[1] = normal;
    getMaterial(id).textures[2] = ao_r_m;

    return id;
}


int
idk::ModelAllocator::createMaterial( const std::string &albedo, const std::string &normal,
                                     const std::string &ao_r_m)
{
    int id = createMaterial();

    getMaterial(id).textures[0] = loadTexture(albedo, m_albedo_config);
    getMaterial(id).textures[1] = loadTexture(normal, m_lightmap_config);
    getMaterial(id).textures[2] = loadTexture(ao_r_m, m_lightmap_config);

    return id;
}


void
idk::ModelAllocator::addUserMaterial( int model, int material, int idx )
{
    getModel(model).user_materials[idx] = material;
}




void
idk::ModelAllocator::clear()
{
    m_materials.clear();
    m_meshes.clear();
    m_models.clear();


    for (auto &allocator: m_mesh_allocators)
    {
        allocator.clear();
    }

    // m_mesh_allocator.clear();
}



void
idk::ModelAllocator::getVertices( int model_id, size_t &num_vertices, std::unique_ptr<idk::Vertex_P_N_T_UV[]> &vertices )
{
    // num_vertices = 0;

    // for (int mesh_id: getModel(model_id).mesh_ids)
    // {
    //     num_vertices += getMesh(mesh_id).numVertices;
    // }

    // vertices = std::make_unique<Vertex_P_N_T_UV[]>(num_vertices);
    // Vertex_P_N_T_UV *buffer = (Vertex_P_N_T_UV *)gl::mapNamedBuffer(m_mesh_allocator.VBO, GL_READ_ONLY);

    // size_t offset = 0;
    // for (int mesh_id: getModel(model_id).mesh_ids)
    // {
    //     auto &mesh = getMesh(mesh_id);

    //     std::memcpy(
    //         vertices.get() + offset,
    //         buffer + mesh.baseVertex,
    //         mesh.numVertices * sizeof(Vertex_P_N_T_UV)
    //     );

    //     offset += mesh.numVertices;
    // }

    // gl::unmapNamedBuffer(m_mesh_allocator.VBO);
}


void
idk::ModelAllocator::getIndices( int model_id, size_t &num_indices, std::unique_ptr<uint32_t[]> &indices )
{
    // num_indices = 0;

    // for (int mesh_id: getModel(model_id).mesh_ids)
    // {
    //     MeshDescriptor &mesh = getMesh(mesh_id); 
    //     num_indices += mesh.numIndices;
    // }

    // indices = std::make_unique<uint32_t[]>(num_indices);
    // uint32_t *buffer = (uint32_t *)gl::mapNamedBuffer(m_mesh_allocator.IBO, GL_READ_ONLY);

    // size_t offset = 0;
    // for (int mesh_id: getModel(model_id).mesh_ids)
    // {
    //     auto &mesh = getMesh(mesh_id);

    //     std::memcpy(
    //         indices.get() + offset,
    //         buffer + mesh.firstIndex,
    //         mesh.numIndices * sizeof(uint32_t)
    //     );

    //     offset += mesh.numIndices;
    // }

    // gl::unmapNamedBuffer(m_mesh_allocator.IBO);
}


