#include "terrain.hpp"
#include "desc.hpp"

#include "../storage/bindings.hpp"
#include "../idk_renderengine.hpp"

#include <libidk/idk_wallocator.hpp>
#include <libidk/idk_random.hpp>


using namespace idk;


#define TEX_W 1024
#define CLIP_W m_terrain_desc.clipmap_size[0]
#define NUM_CLIPS uint32_t(m_terrain_desc.clipmap_size[1])

#define GRASS_TILE_W   8.0
#define GRASS_TILES_XZ 8


struct f32buffer
{
    float data[TEX_W*TEX_W];

    float get( int row, int col )
    {
        return data[TEX_W*(row%TEX_W) + (col%TEX_W)];
    }

    float *operator [] (int i)
    {
        return &(data[i]);
    }
};


namespace
{
    idk::TerrainRenderer::TerrainDesc m_terrain_desc;
    idk::TerrainRenderer::SSBO_Terrain m_terrain;

    std::vector<glm::vec4> m_grass;

    f32buffer           m_readback;
    idk::TextureWrapper m_nmapwrapper;

    int       m_model;
    int       m_clipmaps[2];
    int       m_water_model;
    int       m_grass_model;

    bool      m_should_regen      = false;

    bool      m_terrain_wireframe = false;
    bool      m_water_wireframe   = false;
    bool      m_water_enabled     = true;

    uint32_t  m_terrain_depth;

    uint32_t  m_diff_textures;
    uint32_t  m_norm_textures;
    uint32_t  m_arm_textures;
    uint32_t  m_disp_textures;
    uint32_t  m_grass_textures;

    uint64_t  m_diff_handle;
    uint64_t  m_norm_handle;
    uint64_t  m_arm_handle;
    uint64_t  m_disp_handle;
    uint64_t  m_grass_handle;

    uint32_t  m_height8u_texture;
    uint32_t  m_height_texture;
    uint64_t  m_height_handle;
    uint32_t  m_nmap_texture;
    uint64_t  m_nmap_handle;


    idk::glBufferObject<GL_SHADER_STORAGE_BUFFER> m_SSBO;
    idk::glBufferObject<GL_SHADER_STORAGE_BUFFER> m_SSBOGrass;
    idk::glBufferObject<GL_ATOMIC_COUNTER_BUFFER> m_atomic_buffer;

    idk::glBufferObject<GL_DRAW_INDIRECT_BUFFER>  m_DIB;
    idk::glDrawCmd m_DIB_cmds[16];

    const int DIB_IDX_CLIPMAP = 0;
    const int DIB_IDX_GRASS   = 2;

}




static void
generate_heightmap( idk::RenderEngine &ren )
{
    gl::bindImageTexture(0, m_height_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
    gl::bindImageTexture(1, m_nmap_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);

    {
        auto &program = ren.getBindProgram("terrain-gen");

        gl::memoryBarrier(GL_ALL_BARRIER_BITS);
        program.dispatch(TEX_W/8, TEX_W/8, 1);
    }

    {
        auto &program = ren.getBindProgram("terrain-nmap");

        gl::memoryBarrier(GL_ALL_BARRIER_BITS);
        program.dispatch(TEX_W/8, TEX_W/8, 1);

    }


    gl::memoryBarrier(GL_ALL_BARRIER_BITS);
    gl::bindImageTexture(0, m_height_texture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);

    IDK_GLCALL(
        glGetTextureImage(
        m_height_texture,
            0,
            GL_RED,
            GL_FLOAT,
            TEX_W*TEX_W*sizeof(float),
            &(m_readback.data[0])
        );
    )

    gl::memoryBarrier(GL_ALL_BARRIER_BITS);

}


static void
init_terrain() 
{
    using namespace idk;
    auto &terrain = m_terrain;

    for (int i=0; i<1; i++)
    {
        m_terrain_desc.texscale[i] = glm::vec4(24.0f);
        m_terrain_desc.water_color[i] = glm::vec4(0.0f);
    }

    terrain.height = m_height_handle;
    terrain.nmap   = m_nmap_handle;
    terrain.diff   = m_diff_handle;
    terrain.norm   = m_norm_handle;
    terrain.arm    = m_arm_handle;
    terrain.disp   = m_disp_handle;
}



void
idk::TerrainRenderer::init( idk::RenderEngine &ren, idk::ModelAllocator &MA )
{
    {
        auto VS = idk::glShaderStage("IDKGE/shaders/terrain/clipmap-gpass.vs");
        auto FS = idk::glShaderStage("IDKGE/shaders/terrain/clipmap-gpass.fs");
    
        // auto VS2 = idk::glShaderStage("IDKGE/shaders/terrain/clipmap-shadow.vs");
        // auto FS2 = idk::glShaderStage("IDKGE/shaders/terrain/clipmap-shadow.fs");
    
        ren.createProgram("terrain-clipmap", idk::glShaderProgram(VS, FS));
        ren.createProgram("terrain-shadow",  idk::glShaderProgram(VS, FS));
    }

    ren.createProgram("terrain-gen",  idk::glShaderProgram("IDKGE/shaders/terrain/heightmap-generate.comp"));
    ren.createProgram("terrain-nmap", idk::glShaderProgram("IDKGE/shaders/terrain/heightmap-nmap.comp"));
    ren.createProgram("terrain-grass-gen", idk::glShaderProgram("IDKGE/shaders/terrain/grass-generate.comp"));

    {
        auto VS = idk::glShaderStage("IDKGE/shaders/terrain/water-gpass.vs");
        auto FS = idk::glShaderStage("IDKGE/shaders/terrain/water-gpass.fs");
        ren.createProgram("terrain-water", idk::glShaderProgram(VS, FS));
    }

    {
        auto VS = idk::glShaderStage("IDKGE/shaders/terrain/grass-gpass.vs");
        auto FS = idk::glShaderStage("IDKGE/shaders/terrain/grass-gpass.fs");
        ren.createProgram("terrain-grass", idk::glShaderProgram(VS, FS));
    }

    m_SSBO.init(shader_bindings::SSBO_Terrain);
    m_SSBO.bufferData(sizeof(SSBO_Terrain) + sizeof(TerrainDesc), nullptr, GL_DYNAMIC_DRAW);

    m_SSBOGrass.init(shader_bindings::SSBO_Grass);
    // m_SSBOGrass.bind(shader_bindings::SSBO_Grass);
    m_SSBOGrass.bufferData(4*2048*sizeof(glm::vec4), nullptr, GL_DYNAMIC_DRAW);

    m_atomic_buffer.init(0);
    m_atomic_buffer.bufferData(32*sizeof(uint32_t), nullptr, GL_STATIC_COPY);

    m_DIB.init();
    m_DIB.bufferData(sizeof(m_DIB_cmds), nullptr, GL_DYNAMIC_DRAW);

    m_model = MA.loadModel("IDKGE/resources/terrain/terrain-64x64.idkvi");
    m_grass_model = MA.loadModel("IDKGE/resources/terrain/grass-4.idkvi");

    m_clipmaps[0] = MA.loadModel("IDKGE/resources/terrain/clip0.idkvi");
    m_clipmaps[1] = MA.loadModel("IDKGE/resources/terrain/clip1.idkvi");


    idk::glTextureConfig color_config = {
        .internalformat = GL_RGBA8,
        .format         = GL_RGBA,
        .minfilter      = GL_LINEAR_MIPMAP_LINEAR,
        .magfilter      = GL_LINEAR,
        .wrap_s         = GL_REPEAT,
        .wrap_t         = GL_REPEAT,
        .datatype       = GL_UNSIGNED_BYTE,
        .anisotropic    = GL_TRUE,
        .genmipmap      = GL_TRUE
    };

    idk::glTextureConfig lightmap_config = {
        .internalformat = GL_RGBA8,
        .format         = GL_RGBA,
        .minfilter      = GL_LINEAR_MIPMAP_LINEAR,
        .magfilter      = GL_LINEAR,
        .wrap_s         = GL_REPEAT,
        .wrap_t         = GL_REPEAT,
        .datatype       = GL_UNSIGNED_BYTE,
        .anisotropic    = GL_TRUE,
        .genmipmap      = GL_TRUE
    };

    idk::glTextureConfig height8u_config = {
        .internalformat = GL_RGBA8,
        .format         = GL_RGBA,
        .minfilter      = GL_LINEAR_MIPMAP_LINEAR,
        .magfilter      = GL_LINEAR,
        .wrap_s         = GL_CLAMP_TO_BORDER,
        .wrap_t         = GL_CLAMP_TO_BORDER,
        .datatype       = GL_UNSIGNED_BYTE,
        .anisotropic    = GL_FALSE,
        .genmipmap      = GL_TRUE
    };

    const std::vector<std::string> albedo_paths = {
        "IDKGE/resources/terrain/tex/grass0-diff.jpg",
        "IDKGE/resources/terrain/tex/sand0-diff.jpg",
        "IDKGE/resources/terrain/tex/rocks4-diff.jpg",
        "IDKGE/resources/terrain/tex/rocks5-diff.jpg",
        "IDKGE/resources/terrain/tex/snow1-diff.jpg"
    };

    const std::vector<std::string> normal_paths = {
        "IDKGE/resources/terrain/tex/grass0-norm.jpg",
        "IDKGE/resources/terrain/tex/sand0-norm.jpg",
        "IDKGE/resources/terrain/tex/rocks4-norm.jpg",
        "IDKGE/resources/terrain/tex/rocks5-norm.jpg",
        "IDKGE/resources/terrain/tex/snow1-norm.jpg"
    };

    const std::vector<std::string> arm_paths = {
        "IDKGE/resources/terrain/tex/grass0-arm.jpg",
        "IDKGE/resources/terrain/tex/sand0-arm.jpg",
        "IDKGE/resources/terrain/tex/rocks4-arm.jpg",
        "IDKGE/resources/terrain/tex/rocks5-arm.jpg",
        "IDKGE/resources/terrain/tex/snow1-arm.jpg"
    };

    const std::vector<std::string> disp_paths = {
        "IDKGE/resources/terrain/tex/grass0-disp.jpg",
        "IDKGE/resources/terrain/tex/sand0-disp.jpg",
        "IDKGE/resources/terrain/tex/rocks4-disp.jpg",
        "IDKGE/resources/terrain/tex/rocks5-disp.jpg",
        "IDKGE/resources/terrain/tex/snow1-disp.jpg"
    };

    const std::vector<std::string> grass_paths = {
        "IDKGE/resources/terrain/foliage/grass00.png",
        "IDKGE/resources/terrain/foliage/grass01.png",
        "IDKGE/resources/terrain/foliage/grass02.png",
        "IDKGE/resources/terrain/foliage/grass35.png",
        "IDKGE/resources/terrain/foliage/grass52.png",
        "IDKGE/resources/terrain/foliage/grass53.png"
    };


    m_diff_textures  = gltools::loadTexture2DArray(512, 512, albedo_paths, color_config);
    m_norm_textures  = gltools::loadTexture2DArray(512, 512, normal_paths, lightmap_config);
    m_arm_textures   = gltools::loadTexture2DArray(512, 512, arm_paths,    lightmap_config);
    m_disp_textures  = gltools::loadTexture2DArray(512, 512, disp_paths,   lightmap_config);
    m_grass_textures = gltools::loadTexture2DArray(128, 128, grass_paths,  color_config);

    m_diff_handle  = gl::getTextureHandleARB(m_diff_textures);
    m_norm_handle  = gl::getTextureHandleARB(m_norm_textures);
    m_arm_handle   = gl::getTextureHandleARB(m_arm_textures);
    m_disp_handle  = gl::getTextureHandleARB(m_disp_textures);

    gl::makeTextureHandleResidentARB(m_diff_handle);
    gl::makeTextureHandleResidentARB(m_norm_handle);
    gl::makeTextureHandleResidentARB(m_arm_handle);
    gl::makeTextureHandleResidentARB(m_disp_handle);




    idk::glTextureConfig height_config = {
        .internalformat = GL_R32F,
        .format         = GL_RED,
        .minfilter      = GL_LINEAR,
        .magfilter      = GL_LINEAR,
        .wrap_s         = GL_CLAMP_TO_BORDER,
        .wrap_t         = GL_CLAMP_TO_BORDER,
        .datatype       = GL_FLOAT,
        .anisotropic    = GL_FALSE,
        .genmipmap      = GL_FALSE
    };


    idk::glTextureConfig norm_config = {
        .internalformat = GL_RGBA16F,
        .format         = GL_RGBA,
        .minfilter      = GL_LINEAR,
        .magfilter      = GL_LINEAR,
        .wrap_s         = GL_CLAMP_TO_BORDER,
        .wrap_t         = GL_CLAMP_TO_BORDER,
        .datatype       = GL_FLOAT,
        .anisotropic    = GL_FALSE,
        .genmipmap      = GL_FALSE
    };


    m_terrain_depth = gltools::loadTexture2D(2048, 2024, nullptr, height_config);

    m_height_texture = gltools::loadTexture2D(TEX_W, TEX_W, nullptr, height_config);
    m_nmap_texture   = gltools::loadTexture2D(TEX_W, TEX_W, nullptr, norm_config);

    m_height_handle = gl::getTextureHandleARB(m_height_texture);
    m_nmap_handle   = gl::getTextureHandleARB(m_nmap_texture);


    generate_heightmap(ren);
    init_terrain();
    generateTerrain();

}



static glm::vec3
compute_barycentric( float x, float y, glm::vec2 v1, glm::vec2 v2, glm::vec2 v3 )
{
    float denom = (v2.y-v3.y)*(v1.x-v3.x) + (v3.x-v2.x)*(v1.y-v3.y);

    glm::vec3 weights;
    weights.x = ((v2.y-v3.y)*(x-v3.x) + (v3.x-v2.x)*(y-v3.y)) / denom;
    weights.y = ((v3.y-v1.y)*(x-v3.x) + (v1.x-v3.x)*(y-v3.y)) / denom;
    weights.z = 1 - weights.x - weights.y;
    return weights;
}



float
idk::TerrainRenderer::heightQuery( float x, float z )
{
    auto &terrain = m_terrain;

    glm::mat4 T = m_terrain_desc.transform;
    float xscale = CLIP_W * pow(2, NUM_CLIPS-1);
    float yscale = m_terrain_desc.scale.y;

    glm::vec3 minv  = xscale * glm::vec4(-0.5f, 0.0f, -0.5f, 1.0f);
    glm::vec3 maxv  = xscale * glm::vec4(+0.5f, 0.0f, +0.5f, 1.0f);
    glm::vec3 scale = glm::mat3(T) * glm::vec3(1.0f);

    float x_factor = (x - minv.x) / (maxv.x - minv.x);
    float z_factor = (z - minv.z) / (maxv.z - minv.z);

    x_factor = glm::mod(x_factor, 1.0f);
    z_factor = glm::mod(z_factor, 1.0f);

    int row = int(TEX_W * z_factor);
    int col = int(TEX_W * x_factor);

    if (row != glm::clamp(row, 0, int(TEX_W)))
    {
        return -100000.0f;
    }

    if (col != glm::clamp(col, 0, int(TEX_W)))
    {
        return -100000.0f;
    }

    float tl_h = m_readback.get(row, col);
    float tr_h = m_readback.get(row, col+1);
    float bl_h = m_readback.get(row+1, col);
    float br_h = m_readback.get(row+1, col+1);

    x_factor *= TEX_W;
    z_factor *= TEX_W;

    glm::vec2 tl = glm::vec2(col,   row);
    glm::vec2 tr = glm::vec2(col+1, row);
    glm::vec2 bl = glm::vec2(col,   row+1);
    glm::vec2 br = glm::vec2(col+1, row+1);


    float height = 0.0f;

    if ((x_factor - col) < (1.0f - (z_factor - row)))
    {
        glm::vec3 w0 = compute_barycentric(x_factor, z_factor, bl, tl, tr);
        height = w0[0]*bl_h + w0[1]*tl_h + w0[2]*tr_h;
    }

    else
    {
        glm::vec3 w1 = compute_barycentric(x_factor, z_factor, tr, br, bl);
        height = w1[0]*tr_h + w1[1]*br_h + w1[2]*bl_h;
    }

    height *= m_terrain_desc.scale.x * yscale;
    height += T[3].y;

    return height;
}



glm::vec3
idk::TerrainRenderer::slopeQuery( float x, float z )
{
    float left  = heightQuery(x-0.01, z);
    float right = heightQuery(x+0.01, z);
    float up    = heightQuery(x, z-0.01);
    float down  = heightQuery(x, z+0.01);

    glm::vec3 L = glm::vec3(-0.01f, left,   0.0f);
    glm::vec3 R = glm::vec3(+0.01f, right,  0.0f);
    glm::vec3 U = glm::vec3( 0.0f,  up,    -0.01f);
    glm::vec3 D = glm::vec3( 0.0f,  down,  +0.01f);

    return glm::normalize(glm::cross(R-L, U-D));
}



void
idk::TerrainRenderer::update( idk::RenderEngine &ren, const IDK_Camera &camera, idk::ModelAllocator &MA )
{
    if (m_should_regen)
    {
        generate_heightmap(ren);
        m_should_regen = false;
    }


    // Heightmap
    {
        auto &mesh0  = MA.getModel(m_clipmaps[0]).meshes[0];
        auto &mesh1  = MA.getModel(m_clipmaps[1]).meshes[0];

        m_DIB_cmds[DIB_IDX_CLIPMAP+0] = {
            mesh0.numIndices, uint32_t(1), mesh0.firstIndex, mesh0.baseVertex, 0
        };

        m_DIB_cmds[DIB_IDX_CLIPMAP+1] = {
            mesh1.numIndices, glm::clamp(NUM_CLIPS, uint32_t(0), uint32_t(8)), mesh1.firstIndex, mesh1.baseVertex, 0
        };
    }



    // Grass tiles
    {
        // static glm::vec3 prev_pos = glm::vec3(camera.position);
        // static glm::vec3 curr_pos;

        // curr_pos = glm::vec3(camera.position);

        // int prev_row = int(prev_pos.z / GRASS_TILE_W);
        // int prev_col = int(prev_pos.x / GRASS_TILE_W);

        // int curr_row = int(curr_pos.z / GRASS_TILE_W);
        // int curr_col = int(curr_pos.x / GRASS_TILE_W);

        // if (curr_row != prev_row || curr_col != prev_col)
        // {
        //     uint32_t value = 0;
        //     m_atomic_buffer.bufferSubData(0, sizeof(uint32_t), &value);
        //     m_atomic_buffer.bind(0);

        //     auto &program = ren.getBindProgram("terrain-grass-gen");
        //     program.set_uint("un_tile_w",   GRASS_TILE_W);
        //     program.set_uint("un_tiles_xz", GRASS_TILES_XZ);
        // }

        // prev_pos = curr_pos;
    }


    // Grass DIB
    {
        auto &model = MA.getModelLOD(m_grass_model, 0);
        auto &mesh  = model.meshes[0];

        m_DIB_cmds[DIB_IDX_GRASS] = {
            mesh.numIndices, uint32_t(m_grass.size()), mesh.firstIndex, mesh.baseVertex, 0
        };
    }

    m_DIB.bufferSubData(0, sizeof(m_DIB_cmds), &(m_DIB_cmds[0]));


    m_SSBO.bufferSubData(0, sizeof(SSBO_Terrain), &m_terrain);
    m_SSBO.bufferSubData(sizeof(SSBO_Terrain), sizeof(TerrainDesc), &m_terrain_desc);

}



void
idk::TerrainRenderer::render( idk::RenderEngine &ren, idk::glFramebuffer &buffer_out, float dt, 
                              const IDK_Camera &camera, idk::ModelAllocator &MA )
{
    gl::bindBuffer(GL_DRAW_INDIRECT_BUFFER, m_DIB.ID());
    gl::bindImageTexture(0, m_terrain_depth, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);

    {
        auto &program = ren.getBindProgram("terrain-clipmap");

        program.set_float("un_amp_factor", getWaterTemp().amp_factor);
        program.set_float("un_wav_factor", getWaterTemp().wav_factor);
        program.set_vec4("un_mul_factors", getWaterTemp().mul_factors);
        program.set_int("un_light_id", -1);
        program.set_uint("un_num_instances", 4);


        if (m_terrain_wireframe)
        {
            IDK_GLCALL( glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ); )
        }

        gl::multiDrawElementsIndirect(
            GL_TRIANGLES,
            GL_UNSIGNED_INT,
            (void *)(DIB_IDX_CLIPMAP * sizeof(idk::glDrawCmd)),
            2,
            sizeof(idk::glDrawCmd)
        );

        if (m_terrain_wireframe)
        {
            IDK_GLCALL( glPolygonMode( GL_FRONT_AND_BACK, GL_FILL ); )
        }
    }


    gl::disable(GL_BLEND, GL_CULL_FACE);

    {
        auto &model = MA.getModelLOD(m_grass_model, 0);
        auto &mesh  = model.meshes[0];
        auto &mat   = MA.getMaterial(mesh.material);

        static float alpha = 0.0f;
        alpha += dt;


        auto &program = ren.getBindProgram("terrain-grass");

        program.set_sampler2D("un_albedo", mat.textures[0]);
        program.set_sampler2DArray("un_diff", m_grass_textures);
        program.set_float("un_time", alpha);

        gl::multiDrawElementsIndirect(
            GL_TRIANGLES,
            GL_UNSIGNED_INT,
            (void *)(DIB_IDX_GRASS * sizeof(idk::glDrawCmd)),
            1,
            sizeof(idk::glDrawCmd)
        );
    }



    // Water
    // -----------------------------------------------------------------------------------------
    if (m_water_enabled)
    {
        gl::memoryBarrier(GL_ALL_BARRIER_BITS);
        gl::bindImageTexture(0, m_terrain_depth, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);

        gl::enable(GL_BLEND);
        gl::blendFunci(0, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        gl::blendFunci(1, GL_ONE, GL_ZERO);
        gl::blendFunci(2, GL_ONE, GL_ZERO);


        auto &program = ren.getBindProgram("terrain-water");

        if (m_water_wireframe)
        {
            IDK_GLCALL( glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ); )
        }

        gl::multiDrawElementsIndirect(
            GL_TRIANGLES,
            GL_UNSIGNED_INT,
            (void *)(DIB_IDX_CLIPMAP * sizeof(idk::glDrawCmd)),
            2,
            sizeof(idk::glDrawCmd)
        );

        if (m_water_wireframe)
        {
            IDK_GLCALL( glPolygonMode( GL_FRONT_AND_BACK, GL_FILL ); )
        }

        gl::disable(GL_BLEND);
    }
    // -----------------------------------------------------------------------------------------

    gl::enable(GL_CULL_FACE);
}



uint32_t
idk::TerrainRenderer::getHeightMap()
{
    return m_height_texture;
}


uint32_t
idk::TerrainRenderer::getNormalMap()
{
    return m_nmap_texture;
}



void
idk::TerrainRenderer::renderShadow( idk::RenderEngine &ren, idk::glFramebuffer &buffer_out, idk::ModelAllocator &MA )
{
    using namespace idk;

    gl::bindBuffer(GL_DRAW_INDIRECT_BUFFER, m_DIB.ID());

    auto &program = ren.getBindProgram("terrain-shadow");
    program.set_int("un_light_id", 0);

    for (uint32_t layer=0; layer<5; layer++)
    {
        program.set_uint("un_cascade", layer);

        gl::namedFramebufferTextureLayer(
            buffer_out.m_FBO,
            GL_DEPTH_ATTACHMENT,
            buffer_out.depth_attachment,
            0,
            layer
        );

        gl::multiDrawElementsIndirect(
            GL_TRIANGLES,
            GL_UNSIGNED_INT,
            (void *)(DIB_IDX_CLIPMAP * sizeof(idk::glDrawCmd)),
            2,
            sizeof(idk::glDrawCmd)
        );
    }
}






idk::TerrainRenderer::WaterTemp&
idk::TerrainRenderer::getWaterTemp()
{
    static idk::TerrainRenderer::WaterTemp temp;
    return temp;
}



void
idk::TerrainRenderer::generateTerrain()
{
    m_should_regen = true;
}


void
idk::TerrainRenderer::generateGrass()
{
    glm::vec3 center = glm::vec3(m_terrain_desc.transform[3]);
    glm::vec3 scale  = glm::mat3(m_terrain_desc.transform) * glm::vec3(1.0f);
    glm::vec3 minv   = center - glm::vec3(16.0f); // glm::vec3(0.5f*scale.x);
    glm::vec3 maxv   = center + glm::vec3(16.0f); // glm::vec3(0.5f*scale.x);

    m_grass.clear();

    for (int i=0; i<8*2048; i++)
    {
        if (m_grass.size() >= 4*2048)
        {
            break;
        }

        float x = idk::randf(minv.x, maxv.x);
        float z = idk::randf(minv.z, maxv.z);
        float y = TerrainRenderer::heightQuery(x, z);
        float s = idk::randf(0.6f, 1.0f);

        glm::vec3 N = TerrainRenderer::slopeQuery(x, z);

        if (N.y > 0.9f)
        {
            m_grass.push_back(glm::vec4(x, y, z, s));
        }

    }

    m_SSBOGrass.bufferSubData(0, m_grass.size() * sizeof(glm::vec4), m_grass.data());

    std::cout << "Placed " << m_grass.size() << " grass billboards\n";
}


void
idk::TerrainRenderer::setTerrainWireframe( bool b )
{
    m_terrain_wireframe = b;
}


void
idk::TerrainRenderer::setWaterWireframe( bool b )
{
    m_water_wireframe = b;
}


void
idk::TerrainRenderer::setWaterActive( bool b )
{
    m_water_enabled = b;
}




void
idk::TerrainRenderer::setTerrainTransform( const glm::mat4 &T )
{
    m_terrain_desc.transform = T;
}


idk::TerrainRenderer::TerrainDesc&
idk::TerrainRenderer::getTerrainDesc()
{
    return m_terrain_desc;
}


