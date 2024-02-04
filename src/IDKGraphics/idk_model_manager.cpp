#include "idk_model_manager.hpp"

#include <fstream>
#include <sstream>
#include <set>
#include <filesystem>

#include <libidk/idk_gl.hpp>
#include <libidk/idk_texturegen.hpp>


std::unique_ptr<uint8_t[]> do_thing_r( size_t size, uint8_t value )
{
    auto data = std::unique_ptr<uint8_t[]>(new uint8_t[4*size*size]);

    for (size_t i=0; i<size*size; i++)
    {
        data[4*i+0] = value;
        data[4*i+1] = value;
        data[4*i+2] = value;
        data[4*i+3] = 255;
    }

    return data;
}


std::unique_ptr<uint8_t[]> do_thing_rgba( size_t size, uint8_t r, uint8_t g, uint8_t b, uint8_t a )
{
    auto data = std::unique_ptr<uint8_t[]>(new uint8_t[4*size*size]);

    for (size_t i=0; i<size*size; i++)
    {
        data[4*i+0] = r;
        data[4*i+1] = g;
        data[4*i+2] = b;
        data[4*i+3] = a;
    }

    return data;
}


void
idk::ModelSystem::init()
{
    m_texture_SSBO.init();
    m_texture_SSBO.bind(8);
    m_texture_SSBO.bufferData(4*128*sizeof(GLuint64), nullptr);
    m_texture_handles.resize(4*128);

    m_default_albedo_config = {
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

    m_default_lightmap_config = {
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


    constexpr size_t IMG_W = 16;

    auto data_albedo = texturegen::genRA<uint8_t>(IMG_W, IMG_W, 100, 255 );
    auto data_ao_r_m = texturegen::genRGBA<uint8_t>(IMG_W, IMG_W, 255, 180, 1, 0);
    auto data_normal = texturegen::genRGBA<uint8_t>(IMG_W, IMG_W, 125, 125, 255, 0);
    auto data_emissv = texturegen::genRGBA<uint8_t>(IMG_W, IMG_W, 0, 0, 0, 0);

    m_default_albedo = loadTexture(IMG_W, IMG_W, data_albedo.get(), m_default_albedo_config);
    m_default_ao_r_m = loadTexture(IMG_W, IMG_W, data_ao_r_m.get(), m_default_lightmap_config);
    m_default_normal = loadTexture(IMG_W, IMG_W, data_normal.get(), m_default_lightmap_config);
    m_default_emissv = loadTexture(IMG_W, IMG_W, data_emissv.get(), m_default_lightmap_config);

    // for (size_t i=0; i<=MAX_PLANE_LEVEL; i++)
    // {
    //     m_planes[i] = loadModel("IDKGE/resources/planes/", "plane-" + std::to_string(i));
    // }
}


bool
idk::ModelSystem::_texture_loaded( const std::string &filepath )
{
    return m_texture_IDs[filepath].set == true;
}


int
idk::ModelSystem::_texture_ID( const std::string &filepath )
{
    return m_texture_IDs[filepath].texture_ID == true;
}

idk::glTexture &
idk::ModelSystem::getidkTexture( GLuint gl_id )
{
    return m_textures.get(m_rtexture_IDs[gl_id]);
}


GLuint
idk::ModelSystem::loadTexture( const std::string &filepath, const glTextureConfig &config )
{
    int texture_id = m_textures.create(gltools::loadTexture(filepath, config));
    GLuint gl_id   = m_textures.get(texture_id).ID();

    m_texture_IDs[filepath].set = true;
    m_texture_IDs[filepath].texture_ID = texture_id;
    m_rtexture_IDs[gl_id] = texture_id;

    return gl_id;
}


GLuint
idk::ModelSystem::loadTexture( const std::string &filepath, bool is_lightmap )
{
    auto &config = (is_lightmap) ? m_default_albedo_config : m_default_lightmap_config;
    return loadTexture(filepath, config);
}


GLuint
idk::ModelSystem::loadTexture( size_t w, size_t h, void *data, const glTextureConfig &config )
{
    int texture_id = m_textures.create(gltools::loadTexture2(w, h, data, config));
    GLuint gl_id   = m_textures.get(texture_id).ID();

    m_rtexture_IDs[gl_id] = texture_id;

    return gl_id;
}


GLuint
idk::ModelSystem::getTexture( const std::string &filepath, const glTextureConfig &config )
{
    if (_texture_loaded(filepath) == false)
    {
        loadTexture(filepath, config);
    }

    return _texture_ID(filepath);
}



GLuint
idk::ModelSystem::getTexture( const std::string &filepath )
{
    return _texture_ID(filepath);
}

int
idk::ModelSystem::createMaterial( int albedo, int normal, int ao_r_m, int emissv )
{
    int material_id = m_materials.create();
    idk::Material &material = getMaterial(material_id);

    material.bindless_idx = material_id;

    material.albedo_id   = (albedo == -1) ? m_default_albedo : albedo;
    material.normal_id   = (normal == -1) ? m_default_normal : normal;
    material.arm_id      = (ao_r_m == -1) ? m_default_ao_r_m : ao_r_m;
    material.emission_id = (emissv == -1) ? m_default_emissv : emissv;

    return material_id;
}


int
idk::ModelSystem::loadMaterial( const std::string &root,
                                const std::string &albedo,
                                const std::string &normal,
                                const std::string &ao_rough_metal,
                                const std::string &emission )
{
    int material_id = createMaterial(-1, -1, -1, -1);
    idk::Material &material = getMaterial(material_id);

    if (albedo != "")
        material.albedo_id = loadTexture(root+albedo, m_default_albedo_config);

    if (normal != "")
        material.normal_id = loadTexture(root+normal, m_default_lightmap_config);

    if (ao_rough_metal != "")
        material.arm_id = loadTexture(root+ao_rough_metal, m_default_lightmap_config);

    if (emission != "")
        material.emission_id = loadTexture(root+emission, m_default_lightmap_config);


    // Update SSBO image handles
    // -----------------------------------------------------------------------------------------
    m_texture_handles[4*material_id + 0] = getidkTexture(material.albedo_id).handle();
    m_texture_handles[4*material_id + 1] = getidkTexture(material.normal_id).handle();
    m_texture_handles[4*material_id + 2] = getidkTexture(material.arm_id).handle();
    m_texture_handles[4*material_id + 3] = getidkTexture(material.emission_id).handle();

    m_texture_SSBO.bufferSubData(
        0,
        m_texture_handles.nbytes(),
        m_texture_handles.data()
    );
    // -----------------------------------------------------------------------------------------


    return material_id;
}



void
idk::ModelSystem::model_to_gpu( idk::Model &model )
{
    gl::genVertexArrays(1, &model.VAO);
    gl::genBuffers(1, &model.VBO);
    gl::genBuffers(1, &model.IBO);

    gl::bindVertexArray(model.VAO);
    gl::bindBuffer(GL_ARRAY_BUFFER, model.VBO);

    size_t vertex_size = model.m_vertices->typesize();

    gl::bufferData(
        GL_ARRAY_BUFFER,
        model.m_vertices->nbytes(),
        model.m_vertices->data(),
        GL_STATIC_DRAW
    );

    gl::bindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.IBO);
    gl::bufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        model.m_indices.size() * sizeof(GLuint),
        model.m_indices.data(),
        GL_STATIC_DRAW
    );


    GLuint offset = 0;

    // Position
    gl::vertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertex_size, offset);
    gl::enableVertexAttribArray(0);
    offset += 3 * sizeof(float);

    // Normal
    gl::vertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vertex_size, offset);
    gl::enableVertexAttribArray(1);
    offset += 3 * sizeof(float);

    // Tangent
    gl::vertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, vertex_size, offset);
    gl::enableVertexAttribArray(2);
    offset += 3 * sizeof(float);

    // UV
    gl::vertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, vertex_size, offset);
    gl::enableVertexAttribArray(3);
    offset += 2 * sizeof(float);

    if (model.render_flags & ModelRenderFlag::ANIMATED)
    {
        // Bone IDs
        gl::vertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, vertex_size, offset);
        gl::enableVertexAttribArray(4);
        offset += 4 * sizeof(int);

        // Bone weights
        gl::vertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, vertex_size, offset);
        gl::enableVertexAttribArray(5);
        offset += 4 * sizeof(float);
    }

    gl::bindVertexArray(0);
    gl::bindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


void
idk::ModelSystem::instanced_to_gpu( idk::Model &model, const std::vector<glm::mat4> &transforms )
{
    gl::genVertexArrays(1, &model.VAO);
    gl::genBuffers(1, &model.VBO);
    gl::genBuffers(1, &model.IBO);

    gl::bindVertexArray(model.VAO);
    gl::bindBuffer(GL_ARRAY_BUFFER, model.VBO);


    size_t vertex_size = sizeof(idk::Vertex);

    gl::bufferData(
        GL_ARRAY_BUFFER,
        model.m_vertices->nbytes(),
        model.m_vertices->data(),
        GL_STATIC_DRAW
    );

    gl::bindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.IBO);
    gl::bufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        model.m_indices.size() * sizeof(GLuint),
        model.m_indices.data(),
        GL_STATIC_DRAW
    );


    GLuint index  = 0;
    GLuint offset = 0;

    // Position
    gl::vertexAttribPointer(index, 3, GL_FLOAT, GL_FALSE, vertex_size, offset);
    gl::enableVertexAttribArray(0);
    index  += 1;
    offset += 3 * sizeof(float);

    // Normal
    gl::vertexAttribPointer(index, 3, GL_FLOAT, GL_FALSE, vertex_size, offset);
    gl::enableVertexAttribArray(1);
    index  += 1;
    offset += 3 * sizeof(float);

    // Tangent
    gl::vertexAttribPointer(index, 3, GL_FLOAT, GL_FALSE, vertex_size, offset);
    gl::enableVertexAttribArray(2);
    index  += 1;
    offset += 3 * sizeof(float);

    // UV
    gl::vertexAttribPointer(index, 2, GL_FLOAT, GL_FALSE, vertex_size, offset);
    gl::enableVertexAttribArray(3);
    index  += 1;
    offset += 2 * sizeof(float);


    // Send data for instancing
    model.m_instancedata.init(index, transforms);


    gl::bindVertexArray(0);
    gl::bindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


int
idk::ModelSystem::loadModel( const std::string &root, const std::string &name )
{
    if (m_filepaths[root + name] > 0)
    {
        return m_filepaths[root + name];
    }

    idkvi_header_t header = filetools::readheader(root+name+".txt");

    int model_id = m_models.create();
    idk::Model &model = getModel(model_id);
    model.id = model_id;
    model.name = name;
    model.filepath = root;
    model.filestem = name;

    std::ifstream stream(root+name+".idkvi", std::ios::binary);

    if (header.animated)
    {
        model.render_flags |= ModelRenderFlag::ANIMATED;
        model.animator_id = createAnimator();
        model.m_vertices  = new idk::Buffer<idk::AnimatedVertex>();

        filetools::readidkvi(stream, header, model.m_vertices, model.m_indices);
        filetools::readidka(stream, header, m_animators.get(model.animator_id));
    }

    else
    {
        model.m_vertices = new idk::Buffer<idk::Vertex>();
    
        filetools::readidkvi(stream, header, model.m_vertices, model.m_indices);
    }

    stream.close();

    for (size_t i=0; i<header.num_meshes; i++)
    {
        model.meshes.push_back(idk::Mesh());
        idk::Mesh &mesh   = model.meshes.back();
        mesh.num_indices  = header.m_index_counts[i];

        auto &bitmask  = header.m_bitmasks[i];
        auto &textures = header.m_texture_paths[i];

        std::string albedo = (bitmask & ALBEDO_BIT) ? textures[ALBEDO_IDX] : "";
        std::string normal = (bitmask & NORMAL_BIT) ? textures[NORMAL_IDX] : "";
        std::string ao_r_m = (bitmask & RM_BIT)     ? textures[RM_IDX]     : "";
        std::string emssve = (bitmask & EM_BIT)     ? textures[EM_IDX]     : "";

        idk_printvalue(emssve);

        mesh.material_id = loadMaterial("", albedo, normal, ao_r_m, emssve);
    }

    model_to_gpu(model);

    m_filepaths[root + name] = model_id;

    return model_id;
}

int
idk::ModelSystem::copyModel( int id )
{
    return m_models.create(m_models.get(id));
}



int
idk::ModelSystem::loadTerrainHeightmap( GLuint texture_id )
{
    // int model_id  = copyModel(m_planes[MAX_PLANE_LEVEL]);
    // auto &model   = getModel(model_id);

    // model.render_flags |= ModelRenderFlag::HEIGHTMAPPED;
    // model.terrain_id = m_terrain_models.create();
    // auto &terrain    = m_terrain_models.get(model.terrain_id);

    // idk::glTexture &texture = getidkTexture(texture_id);

    // terrain.heightmap_id     = texture.ID();
    // terrain.heightmap_handle = texture.handle();

    // return model_id;
    return 0;
}



int
idk::ModelSystem::loadTerrainHeightmap( const std::string &filepath )
{
    GLuint gl_id = loadTexture(filepath, m_default_lightmap_config);
    return loadTerrainHeightmap(gl_id);
}


void
idk::ModelSystem::loadTerrainMaterials( int terrain_id, int a, int b )
{
    auto &model   = getModel(terrain_id);
    auto &terrain = m_terrain_models.get(model.terrain_id);

    terrain.num_materials = 2;
    terrain.material_ids[0] = a;
    terrain.material_ids[1] = b;
}


// float
// idk::ModelSystem::queryTerrainHeight( int terrain_id, const glm::mat4 &transform,
//                                       float x, float z )
// {
//     auto &model   = getModel(terrain_id);
//     auto &terrain = m_terrain_models.get(model.terrain_id);
//     auto &texture = getidkTexture(terrain.heightmap_id);

//     float u = (x / terrain.world_scale) * 0.5 + 0.5;
//     float v = (z / terrain.world_scale) * 0.5 + 0.5;

//     return terrain.height_scale * texture.bisample4f(u, v, 4).r;
// }


int
idk::ModelSystem::createAnimator()
{
    return m_animators.create();
}


int
idk::ModelSystem::copyAnimator( int model_id )
{
    int animator_id    = getModel(model_id).animator_id;
    Animator &animator = getAnimator(animator_id);

    return m_animators.create(animator);
}


int
idk::ModelSystem::createInstancedModel( int model_id, const std::vector<glm::mat4> &transforms )
{
    int   id    = m_models.create(m_models.get(model_id));
    auto &model = m_models.get(id);
    model.render_flags |= ModelRenderFlag::INSTANCED;

    instanced_to_gpu(model, transforms);

    return id;
}


int
idk::ModelSystem::createChunkedModel( int model_id, const std::vector<glm::vec4> &positions,
                                      const std::vector<std::vector<glm::mat4>> &transforms )
{
    int  chunked_id = copyModel(model_id);
    auto &model     = m_models.get(chunked_id);
    model.render_flags |= ModelRenderFlag::CHUNKED;

    idk::loadChunked(model, positions, transforms);
    instanced_to_gpu(model, model.m_chunk_transforms);

    return chunked_id;
}

