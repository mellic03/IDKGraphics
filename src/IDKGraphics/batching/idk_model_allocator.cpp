#include "idk_model_allocator.hpp"

#include <libidk/idk_texturegen.hpp>
#include <libidk/GL/idk_gltools.hpp>

#include <cstring>


idk::ModelAllocator::ModelAllocator()
:   m_mesh_allocator (2048 * idk::MEGA, idk::ModelVertexFormat_POS_NORM_TAN_UV)
{
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
    auto data_emissv = texturegen::genRGBA<uint8_t>(IMG_W, IMG_W, 0, 0, 0, 0);

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

    gl::createBuffers(1, &m_draw_indirect_buffer);

    gl::namedBufferData(
        m_draw_indirect_buffer,
        512 * sizeof(idk::glDrawElementsIndirectCommand),
        nullptr,
        GL_DYNAMIC_COPY
    );
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
idk::ModelAllocator::loadModel( const std::string &filepath )
{
    if (m_loaded_model_IDs.contains(filepath))
    {
        return m_loaded_model_IDs[filepath];
    }

    idk::ModelFileHeader header;
    std::vector<idk::MeshFileHeader> meshes;
    std::vector<void *> vertices;
    std::vector<void *> indices;

    idk::ModelFile_read(filepath, header, meshes, vertices, indices);
    idk::ModelDescriptor model_desc;

    for (uint32_t i=0; i<header.num_meshes; i++)
    {
        auto &mesh = meshes[i];

        idk::MeshDescriptor desc = m_mesh_allocator.loadMesh(
            mesh.num_verts * idk::VertexFormat_sizeof(header.vertexformat),
            mesh.num_indices * sizeof(uint32_t),
            vertices[i],
            indices[i]
        );

        desc.material_id = loadMaterial(mesh.bitmask, mesh.textures);
        model_desc.mesh_ids.push_back(createMesh(desc));
    }

    int model_id = createModel(model_desc);
    m_loaded_model_IDs[filepath] = model_id;

    return model_id;
}



void
idk::ModelAllocator::clear()
{
    m_materials.clear();
    m_meshes.clear();
    m_models.clear();

    m_mesh_allocator.clear();
}



void
idk::ModelAllocator::getVertices( int model_id, size_t &num_vertices, std::unique_ptr<uint8_t[]> &vertices )
{
    num_vertices = 0;

    for (int mesh_id: getModel(model_id).mesh_ids)
    {
        MeshDescriptor &mesh = getMesh(mesh_id); 
        num_vertices += mesh.numVertices;
    }


    vertices = std::make_unique<uint8_t[]>(num_vertices * sizeof(Vertex_P_N_T_UV));
    size_t offset = 0;

    uint8_t *buffer = reinterpret_cast<uint8_t *>(
        gl::mapNamedBuffer(m_mesh_allocator.VBO, GL_READ_ONLY)
    );


    for (int mesh_id: getModel(model_id).mesh_ids)
    {
        MeshDescriptor &mesh = getMesh(mesh_id);

        void  *start  = reinterpret_cast<void *>(buffer + mesh.baseVertex);
        size_t nbytes = mesh.numVertices * sizeof(Vertex_P_N_T_UV);

        std::memcpy(
            vertices.get() + offset,
            buffer + mesh.baseVertex * sizeof(Vertex_P_N_T_UV),
            nbytes
        );
    }

    gl::unmapNamedBuffer(m_mesh_allocator.VBO);
}


void
idk::ModelAllocator::getIndices( int model_id, size_t &num_indices, std::unique_ptr<uint32_t[]> &indices )
{
    num_indices = 0;

    for (int mesh_id: getModel(model_id).mesh_ids)
    {
        MeshDescriptor &mesh = getMesh(mesh_id); 
        num_indices += mesh.numIndices;
    }


    indices = std::make_unique<uint32_t[]>(num_indices);
    size_t offset = 0;

    uint32_t *buffer = reinterpret_cast<uint32_t *>(
        gl::mapNamedBuffer(m_mesh_allocator.IBO, GL_READ_ONLY)
    );


    for (int mesh_id: getModel(model_id).mesh_ids)
    {
        MeshDescriptor &mesh = getMesh(mesh_id);

        void  *start  = reinterpret_cast<void *>(buffer + mesh.baseIndex);
        size_t nbytes = mesh.numIndices * sizeof(uint32_t);

        std::memcpy(
            indices.get() + offset,
            buffer + mesh.baseIndex,
            nbytes
        );
    }

    gl::unmapNamedBuffer(m_mesh_allocator.IBO);
}





idk::glDrawElementsIndirectCommand
idk::ModelAllocator::genDrawCommand( int mesh_id )
{
    idk::MeshDescriptor &mesh = getMesh(mesh_id);

    idk::glDrawElementsIndirectCommand cmd = {
        .count           = mesh.numIndices,
        .instanceCount   = 1,
        .firstIndex      = mesh.baseIndex,
        .baseVertex      = mesh.baseVertex,
        .baseInstance    = 0
    };

    return cmd;
}



const std::vector<idk::glDrawElementsIndirectCommand> &
idk::ModelAllocator::genDrawCommands()
{
    static std::vector<idk::glDrawElementsIndirectCommand> commands;
    commands.resize(0);

    static std::unordered_set<GLuint64> handle_set;
    for (GLuint64 handle: handle_set)
    {
        gl::makeTextureHandleNonResidentARB(handle);
    }
    handle_set.clear();


    uint32_t draw_ID = 0;

    for (auto &[model_id, transform]: m_drawlist)
    {
        auto &mesh_ids = getModel(model_id).mesh_ids;

        for (int mesh_id: mesh_ids)
        {
            MeshDescriptor     &mesh = getMesh(mesh_id);
            MaterialDescriptor &material = getMaterial(mesh.material_id);


            uint32_t texture_idx = 0;

            for (auto &texture_id: material.textures)
            {
                TextureDescriptor &texture = getTexture(texture_id);
                IDK_ASSERT("This shouldn't happen!", texture_id != -1);

                m_ModelData.materials[draw_ID][texture_idx] = texture.handle;
                texture_idx += 1;

                handle_set.insert(texture.handle);
            }


            m_ModelData.transforms[draw_ID] = transform;
            draw_ID += 1;


            commands.push_back(genDrawCommand(mesh_id));
        }
    }

    m_ModelData_SSBO.update(m_ModelData);


    for (GLuint64 handle: handle_set)
    {
        gl::makeTextureHandleResidentARB(handle);
    }


    gl::namedBufferSubData(
        m_draw_indirect_buffer,
        0,
        commands.size() * sizeof(idk::glDrawElementsIndirectCommand),
        reinterpret_cast<const void *>(commands.data())
    );

    m_drawlist.resize(0);


    return commands;
}


