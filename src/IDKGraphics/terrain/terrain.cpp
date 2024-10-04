#include "terrain.hpp"
#include "desc.hpp"

#include "../idk_renderengine.hpp"
#include "../storage/bindings.hpp"
#include "../noise/noise.hpp"
#include "../time/time.hpp"

#include <libidk/idk_wallocator.hpp>
#include <libidk/idk_random.hpp>
#include <IDKThreading/IDKThreading.hpp>


using namespace idk;


#define TEX_W 1024
#define CLIP_W m_terrain_desc.clipmap_size[0]
#define NUM_CLIPS uint32_t(m_terrain_desc.clipmap_size[1])

#define WATER_OCTAVES 512

#define GRASS_TILE_W   4.0
#define GRASS_TILES_XZ 8


struct f32buffer
{
    float data[TEX_W*TEX_W];

    float sampleNearest_i32( int x, int y )
    {
        x %= TEX_W;
        y %= TEX_W;

        return data[TEX_W*y + x];
    }

    float sampleNearest_f32( float u, float v )
    {
        int x = int(u * TEX_W) % TEX_W;
        int y = int(v * TEX_W) % TEX_W;

        return data[TEX_W*y + x];
    }

    float sampleBillinear( float u, float v )
    {
        u *= TEX_W;
        v *= TEX_W;

        float x_factor = u - floor(u);
        float y_factor = v - floor(v);

        int x0 = int(u+0) % TEX_W;
        int x1 = int(u+1) % TEX_W;
        int y0 = int(v+0) % TEX_W;
        int y1 = int(v+1) % TEX_W;

        float u00 = sampleNearest_i32(x0, y0);
        float u01 = sampleNearest_i32(x1, y0);
        float u10 = sampleNearest_i32(x0, y1);
        float u11 = sampleNearest_i32(x1, y1);

        float u0 = glm::mix(u00, u01, x_factor);
        float u1 = glm::mix(u10, u11, x_factor);

        return glm::mix(u0, u1, y_factor);
    }
};


struct f32bufferxyz
{
    glm::vec3 data[TEX_W*TEX_W];

    glm::vec3 sampleNearest_i32( int x, int y )
    {
        x %= TEX_W;
        y %= TEX_W;

        return data[TEX_W*y + x];
    }

    glm::vec3 sampleNearest_f32( float u, float v )
    {
        int x = int(u * TEX_W) % TEX_W;
        int y = int(v * TEX_W) % TEX_W;

        return data[TEX_W*y + x];
    }

    glm::vec3 sampleBillinear( float u, float v )
    {
        u *= TEX_W;
        v *= TEX_W;

        float x_factor = (u - floor(u)) / (ceil(u) - floor(u));
        float y_factor = (v - floor(v)) / (ceil(v) - floor(v));

        int x = int(u) % TEX_W;
        int y = int(v) % TEX_W;

        glm::vec3 u00 = sampleNearest_i32(x+0, y+0);
        glm::vec3 u01 = sampleNearest_i32(x+1, y+0);
        glm::vec3 u10 = sampleNearest_i32(x+0, y+1);
        glm::vec3 u11 = sampleNearest_i32(x+1, y+1);

        glm::vec3 u0 = glm::mix(u00, u01, x_factor);
        glm::vec3 u1 = glm::mix(u10, u11, x_factor);

        return glm::mix(u0, u1, y_factor);
    }

};


namespace
{
    idk::TerrainRenderer::TerrainDesc m_terrain_desc;
    idk::TerrainRenderer::SSBO_Terrain m_terrain;

    std::vector<glm::vec2> m_water_dirs;
    std::vector<glm::vec2> m_grass;

    f32buffer           m_readback;
    f32bufferxyz        m_nmap_readback;
    f32buffer           m_grass_readback;
    idk::TextureWrapper m_nmapwrapper;

    int       m_model;
    int       m_clipmaps[2];
    int       m_water_model;
    int       m_grass_model;

    bool      m_should_regen      = false;

    bool      m_terrain_wireframe = false;
    bool      m_water_wireframe   = false;
    bool      m_water_enabled     = true;
    bool      m_grass_enabled     = true;

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
    gl::bindImageTexture(0, m_height_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
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
    gl::bindImageTexture(0, m_height_texture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);

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

    IDK_GLCALL(
        glGetTextureImage(
        m_height_texture,
            0,
            GL_GREEN,
            GL_FLOAT,
            TEX_W*TEX_W*sizeof(float),
            &(m_grass_readback.data[0])
        );
    )

    IDK_GLCALL(
        glGetTextureImage(
        m_nmap_texture,
            0,
            GL_RGB,
            GL_FLOAT,
            TEX_W*TEX_W*sizeof(glm::vec3),
            &(m_nmap_readback.data[0])
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
    
        auto VS2 = idk::glShaderStage("IDKGE/shaders/terrain/clipmap-shadow.vs");
        auto FS2 = idk::glShaderStage("IDKGE/shaders/terrain/clipmap-shadow.fs");
    
        ren.createProgram("terrain-clipmap", idk::glShaderProgram(VS, FS));
        ren.createProgram("terrain-shadow",  idk::glShaderProgram(VS2, FS2));
    }

    ren.createProgram("terrain-gen",  idk::glShaderProgram("IDKGE/shaders/terrain/heightmap-generate.comp"));
    ren.createProgram("terrain-nmap", idk::glShaderProgram("IDKGE/shaders/terrain/heightmap-nmap.comp"));
    ren.createProgram("terrain-grass-gen", idk::glShaderProgram("IDKGE/shaders/terrain/grass-generate.comp"));

    {
        auto VS  = idk::glShaderStage("IDKGE/shaders/terrain/water-gpass.vs");
        auto FS  = idk::glShaderStage("IDKGE/shaders/terrain/water-gpass.fs");
        ren.createProgram("terrain-water", idk::glShaderProgram(VS, FS));
    }

    {
        auto VS = idk::glShaderStage("IDKGE/shaders/terrain/grass-gpass.vs");
        auto FS = idk::glShaderStage("IDKGE/shaders/terrain/grass-gpass.fs");
        ren.createProgram("grass-gpass", idk::glShaderProgram(VS, FS));
    }

    m_SSBO.init(shader_bindings::SSBO_Terrain);
    m_SSBO.bufferData(
        sizeof(SSBO_Terrain) + sizeof(TerrainDesc) + WATER_OCTAVES*sizeof(glm::vec2),
        nullptr,
        GL_DYNAMIC_DRAW
    );

    for (int i=0; i<WATER_OCTAVES; i++)
    {
        glm::vec2 dir = idk::randvec2(-1.0f, +1.0f);
        m_water_dirs.push_back(dir);
    }

    m_SSBO.bufferSubData(
        sizeof(SSBO_Terrain) + sizeof(TerrainDesc),
        WATER_OCTAVES*sizeof(glm::vec2),
        m_water_dirs.data()
    );


    m_SSBOGrass.init(shader_bindings::SSBO_Grass);
    // m_SSBOGrass.bind(shader_bindings::SSBO_Grass);
    m_SSBOGrass.bufferData(128*4096*sizeof(glm::vec2), nullptr, GL_DYNAMIC_DRAW);

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


    m_diff_textures  = gltools::loadTexture2DArray(1024, 1024, albedo_paths, color_config);
    m_norm_textures  = gltools::loadTexture2DArray(1024, 1024, normal_paths, lightmap_config);
    m_arm_textures   = gltools::loadTexture2DArray(1024, 1024, arm_paths,    lightmap_config);
    m_disp_textures  = gltools::loadTexture2DArray(1024, 1024, disp_paths,   lightmap_config);
    m_grass_textures = gltools::loadTexture2DArray(128,  128,  grass_paths,  color_config);

    m_diff_handle  = gl::getTextureHandleARB(m_diff_textures);
    m_norm_handle  = gl::getTextureHandleARB(m_norm_textures);
    m_arm_handle   = gl::getTextureHandleARB(m_arm_textures);
    m_disp_handle  = gl::getTextureHandleARB(m_disp_textures);

    gl::makeTextureHandleResidentARB(m_diff_handle);
    gl::makeTextureHandleResidentARB(m_norm_handle);
    gl::makeTextureHandleResidentARB(m_arm_handle);
    gl::makeTextureHandleResidentARB(m_disp_handle);




    idk::glTextureConfig height_config = {
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



static glm::vec2
world_to_uv( float x, float z )
{
    auto &terrain = m_terrain_desc;

    float xscale = glm::length(glm::vec3(terrain.transform[0]));
    float yscale = terrain.scale.y;

    glm::vec3 minv  = xscale * glm::vec4(-0.5f, 0.0f, -0.5f, 1.0f);
    glm::vec3 maxv  = xscale * glm::vec4(+0.5f, 0.0f, +0.5f, 1.0f);

    float u = (x - minv.x) / (maxv.x - minv.x);
    float v = (z - minv.z) / (maxv.z - minv.z);

    return glm::vec2(u, v);
}



float
idk::TerrainRenderer::heightQuery( float x, float z )
{
    glm::vec2 uv = world_to_uv(x, z);

    float height  = m_readback.sampleBillinear(uv.x, uv.y);
          height *= m_terrain_desc.scale.x * m_terrain_desc.scale.y;
          height += m_terrain_desc.transform[3].y;

    return height;
}




static glm::vec3
waterComputeHeight( float time, float x, float z )
{
    auto NF = m_terrain_desc.water;

    float xscale = m_terrain_desc.water_scale[0];
    float yscale = m_terrain_desc.water_scale[1];
    float tscale = m_terrain_desc.water_scale[2];
    float wscale = m_terrain_desc.water_scale[3];

    glm::vec2 gradient = glm::vec2(0.0f);

    float t        = tscale * time;
    float height   = 0.0;
    float a        = yscale;
    float w        = 1.0 / xscale;

    int waves = glm::clamp(64, 1, WATER_OCTAVES);

    for (int i=0; i<waves; i++)
    {
        glm::vec2 dir = m_water_dirs[i];
        float xz = glm::dot(glm::vec2(x, z), dir);

        height += a * sin(w*xz + t);
        gradient += a * w * dir * cos(w*xz + t);

        a *= NF.amp;
        w *= NF.wav;
    }

    height += m_terrain_desc.water_pos.y;

    return glm::vec3(height, gradient);
}




float
idk::TerrainRenderer::waterHeightQuery( float x, float z )
{
    glm::vec2 uv = 2048.0f * world_to_uv(x, z);

    float t  = idk::getTime();
    float dt = idk::getDeltaTime();

    glm::vec3 pdh = waterComputeHeight(t-dt, uv.x, uv.y);

    uv.x -= pdh[1];
    uv.y -= pdh[2];

    pdh = waterComputeHeight(t, uv.x, uv.y);
    
    return pdh[0];
}



glm::vec3
idk::TerrainRenderer::slopeQuery( float x, float z )
{
    glm::vec2 uv = world_to_uv(x, z);
    return glm::normalize(m_nmap_readback.sampleBillinear(uv.x, uv.y));
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
            mesh1.numIndices, std::max(NUM_CLIPS, uint32_t(0)), mesh1.firstIndex, mesh1.baseVertex, 0
        };
    }

    // Grass tiles
    {
        static int row = 32;
        static int col = 32;

        int curr_row = int(ren.getCamera().position.x / GRASS_TILE_W);
        int curr_col = int(ren.getCamera().position.z / GRASS_TILE_W);

        if (row != curr_row || col != curr_col)
        {
            generateGrass(ren.getCamera().position);
        }

        row = curr_row;
        col = curr_col;
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
idk::TerrainRenderer::render( idk::RenderEngine &ren, float dt,
                              idk::glFramebuffer &buffer_out, idk::glFramebuffer &tmp_depth, 
                              const IDK_Camera &camera, idk::ModelAllocator &MA )
{
    gl::bindBuffer(GL_DRAW_INDIRECT_BUFFER, m_DIB.ID());

    {
        auto &program = ren.getBindProgram("terrain-clipmap");
        program.set_uint("un_light_id", -1);
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

    // Grass
    // -----------------------------------------------------------------------------------------
    if (m_grass_enabled)
    {
        gl::disable(GL_BLEND);

        auto &model = MA.getModelLOD(m_grass_model, 0);
        auto &mesh  = model.meshes[0];
        auto &mat   = MA.getMaterial(mesh.material);

        auto &program = ren.getBindProgram("grass-gpass");
        program.set_sampler2D("un_albedo", mat.textures[0]);
        program.set_sampler2DArray("un_diff", m_grass_textures);

        gl::multiDrawElementsIndirect(
            GL_TRIANGLES,
            GL_UNSIGNED_INT,
            (void *)(DIB_IDX_GRASS * sizeof(idk::glDrawCmd)),
            1,
            sizeof(idk::glDrawCmd)
        );
    }
    // -----------------------------------------------------------------------------------------


    // Water
    // -----------------------------------------------------------------------------------------
    if (m_water_enabled)
    {
        gl::enable(GL_BLEND, GL_CULL_FACE);
        gl::cullFace(GL_BACK);
        gl::blendFunci(0, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        gl::blendFunci(1, GL_ONE, GL_ZERO);
        gl::blendFunci(2, GL_ONE, GL_ZERO);

        IDK_GLCALL(
            glBlitNamedFramebuffer(
                buffer_out.m_FBO,
                tmp_depth.m_FBO,
                0, 0, ren.width(), ren.height(),
                0, 0, ren.width(), ren.height(),
                GL_DEPTH_BUFFER_BIT,
                GL_NEAREST
            );
        )

        auto &program = ren.getBindProgram("terrain-water");
        program.set_sampler2D("un_tmp_depth", tmp_depth.attachments[0]);

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




void
idk::TerrainRenderer::renderGrass( idk::RenderEngine &ren, idk::glFramebuffer &buffer_out, 
                                   const IDK_Camera &camera, idk::ModelAllocator &MA )
{
    // gl::bindBuffer(GL_DRAW_INDIRECT_BUFFER, m_DIB.ID());
    // gl::bindVertexArray(MA.getVAO());
    // gl::disable(GL_BLEND, GL_CULL_FACE);

    // auto &model = MA.getModelLOD(m_grass_model, 0);
    // auto &mesh  = model.meshes[0];
    // auto &mat   = MA.getMaterial(mesh.material);

    // auto &program = ren.getBindProgram("grass-gpass");
    // program.set_sampler2D("un_albedo", mat.textures[0]);
    // program.set_sampler2DArray("un_diff", m_grass_textures);

    // gl::multiDrawElementsIndirect(
    //     GL_TRIANGLES,
    //     GL_UNSIGNED_INT,
    //     (void *)(DIB_IDX_GRASS * sizeof(idk::glDrawCmd)),
    //     1,
    //     sizeof(idk::glDrawCmd)
    // );
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
    program.set_uint("un_light_id", 0);

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



static void
generate_grass_tile( float xmin, float zmin, float div, std::vector<glm::vec2> &out )
{
    float step_size = GRASS_TILE_W / div;

    for (float z=zmin; z<zmin+GRASS_TILE_W; z+=step_size)
    {
        for (float x=xmin; x<xmin+GRASS_TILE_W; x+=step_size)
        {
            glm::vec2 uv = world_to_uv(x, z);

            if (m_grass_readback.sampleBillinear(uv.x, uv.y) < 0.65f)
            {
                continue;
            }

            float th = idk::TerrainRenderer::heightQuery(x, z);
            float wh = idk::TerrainRenderer::waterHeightQuery(x, z);

            if (wh > th)
            {
                continue;
            }

            if (idk::TerrainRenderer::slopeQuery(x, z).y < 0.95f)
            {
                continue;
            }

            glm::vec2 offset = idk::noise::BlueRG(4.0f*uv.x, 4.0f*uv.y) * 2.0f - 1.0f;
            glm::vec2 pos    = glm::vec2(x, z);

            pos.x += step_size * offset.x;
            pos.y += step_size * offset.y;

            out.push_back(pos);
        }
    }
}


void
idk::TerrainRenderer::generateGrass( const glm::vec3 &pos )
{
    static std::vector<glm::vec2> positions;
    static int count = 0;

    if (count > 0)
    {
        return;
    }

    count += 1;

    // 1               1               1
    // 1       1       1       1       1
    // 1   1   1   1   1   1   1   1   1
    // 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1

    idk::ThreadPool::createTask(
        [pos]()
        {
            glm::vec3 center = glm::floor(pos / float(GRASS_TILE_W)) * float(GRASS_TILE_W);
            glm::vec3 scale  = glm::mat3(m_terrain_desc.transform) * glm::vec3(1.0f);
            // glm::vec3 minv   = center - glm::vec3(32.0f);
            // glm::vec3 maxv   = center + glm::vec3(32.0f);

            int tile_w = int(GRASS_TILE_W);
            int hw = 16;

            for (int row=-hw; row<=+hw; row++)
            {
                for (int col=-hw; col<=+hw; col++)
                {
                    float x = center.x + float(col*tile_w);
                    float z = center.z + float(row*tile_w);

                    float dist = glm::distance(glm::vec2(x, z), glm::vec2(center.x, center.z));
                          dist = glm::clamp(dist, 0.0f, 128.0f);

                    float div = 1.0f;

                         if (dist <  4*GRASS_TILE_W)  div = 6;
                    else if (dist <  8*GRASS_TILE_W)  div = 5;
                    else if (dist < 16*GRASS_TILE_W)  div = 4;
                    else if (dist < 32*GRASS_TILE_W)  div = 3;
                    // else if (dist < 30*GRASS_TILE_W)  div = 2;
                    // else if (dist < 38*GRASS_TILE_W)  div = 1;
                    // else if (dist < 46*GRASS_TILE_W)  div = 1;

                    generate_grass_tile(x, z, div, positions);
                }
            }
        },
        []()
        {
            m_grass = positions;
            positions.clear();
            m_SSBOGrass.bufferSubData(0, m_grass.size() * sizeof(glm::vec2), m_grass.data());
            std::cout << "Placed " << m_grass.size() << " grass billboards\n";

            count = 0;
        }
    );

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


