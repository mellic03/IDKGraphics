#include "idk_renderengine.hpp"
#include "idk_render_settings.hpp"

#include "lighting/idk_shadowcascade.hpp"
#include "render/idk_initflags.hpp"
#include "render/idk_vxgi.hpp"
#include "render/cubemap.hpp"

#include "particle/particle_system.hpp"
#include "terrain/terrain.hpp"
#include "storage/bindings.hpp"
#include "noise/noise.hpp"
#include "time/time.hpp"

#include "renderstage/ssr.hpp"

#include <libidk/GL/idk_glShaderStage.hpp>
#include <libidk/idk_image.hpp>
#include <libidk/idk_texturegen.hpp>
#include <libidk/idk_log.hpp>


static float delta_time = 1.0f;


static glm::vec2 halton_sequence[128];

// static glm::vec2 taa_sequence[5];

// static glm::vec2 halton_sequence[16] {
//     glm::vec2(0.500000, 0.333333),
//     glm::vec2(0.250000, 0.666667),
//     glm::vec2(0.750000, 0.111111),
//     glm::vec2(0.125000, 0.444444),
//     glm::vec2(0.625000, 0.777778),
//     glm::vec2(0.375000, 0.222222),
//     glm::vec2(0.875000, 0.555556),
//     glm::vec2(0.062500, 0.888889),
//     glm::vec2(0.562500, 0.037037),
//     glm::vec2(0.312500, 0.370370),
//     glm::vec2(0.812500, 0.703704),
//     glm::vec2(0.187500, 0.148148),
//     glm::vec2(0.687500, 0.481481),
//     glm::vec2(0.437500, 0.814815),
//     glm::vec2(0.937500, 0.259259),
//     glm::vec2(0.031250, 0.592593)
// };


float create_halton( int idx, int base )
{
    float f = 1;
    float r = 0;
    int current = idx;

    do
    {
        f = f / base;
        r = r + f * (current % base);
        current = glm::floor(current / base);
    } while (current > 0);

    return r;
}


idk::RenderEngine::RenderEngine( const std::string &name, int w, int h, int gl_major,
                                 int gl_minor, uint32_t flags )
:
    m_windowsys      (name.c_str(), w, h, gl_major, gl_minor, flags),
    m_winsize        (w, h),
    m_resolution     (w, h),
    m_rendersettings (new RenderSettings),
    m_config         (new RenderConfig)
{
    gl::enable(GL_DEPTH_TEST, GL_CULL_FACE);

    init_all(name, w, h);
    resize(w, h);

    idk::ParticleSystem::init(*this);
    idk::TerrainRenderer::init(*this, m_model_allocator);


    // taa_sequence[0] = glm::vec2(-1, -1);
    // taa_sequence[1] = glm::vec2(-1, +1);
    // taa_sequence[2] = glm::vec2( 0,  0);
    // taa_sequence[3] = glm::vec2(+0, -1);
    // taa_sequence[4] = glm::vec2(+0, +1);

    // Generate Halton sequence
    for (int i=0; i<128; i++)
    {
        halton_sequence[i].x = create_halton(i+1, 2);
        halton_sequence[i].y = create_halton(i+1, 3);
    }

    // {
    //     idk::glTextureConfig config = {
    //         .internalformat = GL_RGBA16F,
    //         .format         = GL_RGBA,
    //         .minfilter      = GL_LINEAR,
    //         .magfilter      = GL_LINEAR,
    //         .datatype       = GL_FLOAT,
    //         .genmipmap      = GL_FALSE
    //     };

    //     m_postprocess_fb = createFramebuffer(1.0f, 1.0f, 1);
    //     auto &fb = getFramebuffer(m_postprocess_fb);
    //           fb.addAttachment(0, new FramebufferAttachment(config));
    // }

    // m_render_stages.push_back(dynamic_cast<idk::RenderStage*>(new idk::RenderStageB));

    // for (auto *RS: m_render_stages)
    // {
    //     RS->init(*this);
    // }
}





void
idk::RenderEngine::init_screenquad()
{
    float quad_vertices[] = {
      -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,
      -1.0f, -1.0f,  0.0f,  0.0f,  0.0f,
       1.0f, -1.0f,  0.0f,  1.0f,  0.0f,

      -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,
       1.0f, -1.0f,  0.0f,  1.0f,  0.0f,
       1.0f,  1.0f,  0.0f,  1.0f,  1.0f
    };

    // Send screen quad to GPU
    // ------------------------------------------------------------------------------------
    gl::genVertexArrays(1, &m_quad_VAO);
    gl::genBuffers(1, &m_quad_VBO);

    gl::bindVertexArray(m_quad_VAO);
    gl::bindBuffer(GL_ARRAY_BUFFER, m_quad_VBO);
    gl::bufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

    gl::vertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), 0);
    gl::enableVertexAttribArray(0);

    gl::vertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), 3*sizeof(float));
    gl::enableVertexAttribArray(1);
    // ------------------------------------------------------------------------------------
}


void
idk::RenderEngine::compileShaders()
{
    createProgram("text",         glShaderProgram("IDKGE/shaders/text.comp"));
    createProgram("button-rect",  "IDKGE/shaders/", "button-rect.vs", "button-rect.fs");
    createProgram("ui-text",      "IDKGE/shaders/", "ui-text.vs", "ui-text.fs");
    createProgram("ui-quad",      "IDKGE/shaders/", "ui-quad.vs", "ui-quad.fs");
    // createProgram("ui-image",     "IDKGE/shaders/", "ui-image.vs", "ui-image.fs");

    createProgram("overlay",      glShaderProgram("IDKGE/shaders/overlay.comp"));
    createProgram("overlay-fill", glShaderProgram("IDKGE/shaders/overlay-fill.comp"));

    createProgram("background", "IDKGE/shaders/", "screenquad.vs", "deferred/background.fs");

    createProgram("gpass",           "IDKGE/shaders/deferred/", "gpass.vs", "gpass.fs");
    createProgram("gpass-decal",     "IDKGE/shaders/deferred/", "gpass.vs", "gpass-decal.fs");

    createProgram("shadowcascade-split", glShaderProgram("IDKGE/shaders/shadowcascade-split.comp"));

    // Probe-based GI
    // -----------------------------------------------------------------------------------------
    // idk::glShaderStage VS_cubemap("IDKGE/shaders/GI/envprobe-cubemap.vs");
    // idk::glShaderStage FS_cubemap("IDKGE/shaders/GI/envprobe-cubemap.fs");
    // createProgram("envprobe-cubemap", idk::glShaderProgram(VS_cubemap, FS_cubemap));

    // idk::glShaderStage VS_cubemap_bg("IDKGE/shaders/GI/envprobe-background.vs");
    // idk::glShaderStage FS_cubemap_bg("IDKGE/shaders/GI/envprobe-background.fs");
    // createProgram("envprobe-background", idk::glShaderProgram(VS_cubemap_bg, FS_cubemap_bg));

    // idk::glShaderStage VS_convolve_cubemap("IDKGE/shaders/GI/convolve-cubemap.vs");
    // idk::glShaderStage FS_convolve_cubemap("IDKGE/shaders/GI/convolve-cubemap.fs");
    // createProgram("convolve-cubemap", idk::glShaderProgram(VS_convolve_cubemap, FS_convolve_cubemap));

    // idk::glShaderStage VS_radiance("IDKGE/shaders/deferred/radiance.vs");
    // idk::glShaderStage FS_radiance("IDKGE/shaders/deferred/radiance.fs");
    // createProgram("radiance", idk::glShaderProgram(VS_radiance, FS_radiance));
    // -----------------------------------------------------------------------------------------


    // Deferred lightsources
    // -----------------------------------------------------------------------------------------
    idk::glShaderStage VS_Dirlight("IDKGE/shaders/deferred/dirlight.vs");
    idk::glShaderStage FS_Dirlight("IDKGE/shaders/deferred/dirlight.fs");
    idk::glShaderStage FS_DirlightVol("IDKGE/shaders/deferred/dirlight-volumetric.fs");
    createProgram("deferred-dirlight", idk::glShaderProgram(VS_Dirlight, FS_Dirlight));
    createProgram("dirlight-volumetric", idk::glShaderProgram(VS_Dirlight, FS_DirlightVol));


    idk::glShaderStage VS_Pointlight("IDKGE/shaders/deferred/pointlight.vs");
    idk::glShaderStage FS_Pointlight("IDKGE/shaders/deferred/pointlight.fs");
    createProgram("deferred-pointlight", idk::glShaderProgram(VS_Pointlight, FS_Pointlight));

    idk::glShaderStage VS_Spotlight("IDKGE/shaders/deferred/spotlight.vs");
    idk::glShaderStage FS_Spotlight("IDKGE/shaders/deferred/spotlight.fs");
    createProgram("deferred-spotlight", idk::glShaderProgram(VS_Spotlight, FS_Spotlight));
    // -----------------------------------------------------------------------------------------


    idk::glShaderStage VS_SSAO("IDKGE/shaders/screenquad.vs");
    idk::glShaderStage FS_SSAO("IDKGE/shaders/post/SSAO.fs");
    createProgram("SSAO", idk::glShaderProgram(VS_SSAO, FS_SSAO));
    createProgram("SSAO_composite", "IDKGE/shaders/", "screenquad.vs", "post/SSAO_composite.fs");


    createProgram("blit", "IDKGE/shaders/", "screenquad.vs", "post/blit.fs");
    createProgram("blit-volumetrics", "IDKGE/shaders/", "screenquad.vs", "post/blit-volumetrics.fs");
    createProgram("blit-foliage", "IDKGE/shaders/", "screenquad.vs", "post/blit-foliage.fs");
    createProgram("composite", "IDKGE/shaders/", "screenquad.vs", "post/composite.fs");

    createProgram("dirshadow-indirect", "IDKGE/shaders/", "dirshadow-indirect.vs", "dirshadow.fs");
    createProgram("SSR", "IDKGE/shaders/", "screenquad.vs", "post/SSR.fs");
    createProgram("SSR-downsample", glShaderProgram("IDKGE/shaders/post/SSR-downsample.comp"));

    createProgram("SSS", "IDKGE/shaders/", "screenquad.vs", "post/SSS.fs");


    createProgram("bloom-first", "IDKGE/shaders/", "screenquad.vs", "post/bloom-first.fs");
    createProgram("bloom-down",  "IDKGE/shaders/", "screenquad.vs", "post/bloom-down.fs");
    createProgram("bloom-up",    "IDKGE/shaders/", "screenquad.vs", "post/bloom-up.fs");

    createProgram("screenquad", "IDKGE/shaders/", "screenquad.vs", "screenquad.fs");
    createProgram("filter-gaussian", "IDKGE/shaders/", "screenquad.vs", "post/filter-gaussian.fs");
    createProgram("filter-unsharp", "IDKGE/shaders/", "screenquad.vs", "post/filter-unsharp.fs");

    // createProgram("fxaa",           "IDKGE/shaders/", "screenquad.vs", "post/aa-fxaa.fs");
    createProgram("TAA", "IDKGE/shaders/", "screenquad.vs", "post/aa-taa.fs");

    createProgram("colorgrade",     "IDKGE/shaders/", "screenquad.vs", "post/colorgrade.fs");
    createProgram("alpha-0-1",      "IDKGE/shaders/", "screenquad.vs", "post/alpha-0-1.fs");

}


void
idk::RenderEngine::_recompileShaders()
{
    for (auto &program: m_programs)
    {
        program.recompile();
    }
}


void
idk::RenderEngine::recompileShaders()
{
    m_should_recompile = true;
}



void
idk::RenderEngine::init_framebuffers( int w, int h )
{
    w = glm::clamp(w, 128, 4096);
    h = glm::clamp(h, 128, 4096);

    idk::glTextureConfig config = {
        .internalformat = GL_RGB10_A2,
        .format         = GL_RGBA,
        .minfilter      = GL_LINEAR,
        .magfilter      = GL_LINEAR,
        .datatype       = GL_UNSIGNED_BYTE,
        .genmipmap      = GL_FALSE
    };

    idk::glTextureConfig SSR_config = {
        .internalformat = GL_RGBA16F,
        .format         = GL_RGBA,
        .minfilter      = GL_LINEAR,
        .magfilter      = GL_LINEAR,
        .datatype       = GL_FLOAT,
        .genmipmap      = GL_FALSE
    };

    idk::glTextureConfig foliage_config = {
        .internalformat = GL_RGBA8,
        .format         = GL_RGBA,
        .minfilter      = GL_LINEAR,
        .magfilter      = GL_LINEAR,
        .datatype       = GL_UNSIGNED_BYTE,
        .genmipmap      = GL_FALSE
    };

    idk::glTextureConfig radiance_config = {
        .internalformat = GL_RGBA16F,
        .format         = GL_RGBA,
        .minfilter      = GL_LINEAR,
        .magfilter      = GL_LINEAR,
        .datatype       = GL_FLOAT,
        .genmipmap      = GL_FALSE
    };

    idk::glTextureConfig vol_config = {
        .internalformat = GL_RGBA8,
        .format         = GL_RGBA,
        .minfilter      = GL_LINEAR,
        .magfilter      = GL_LINEAR,
        .datatype       = GL_UNSIGNED_BYTE,
        .genmipmap      = GL_FALSE
    };

    idk::glTextureConfig SSAO_config = {
        .internalformat = GL_R8,
        .format         = GL_RED,
        .minfilter      = GL_NEAREST,
        .magfilter      = GL_LINEAR,
        .datatype       = GL_UNSIGNED_BYTE,
        .genmipmap      = GL_FALSE
    };

    idk::glTextureConfig shadow_config = {
        .internalformat = GL_R8,
        .format         = GL_RED,
        .minfilter      = GL_LINEAR,
        .magfilter      = GL_LINEAR,
        .datatype       = GL_FLOAT,
        .genmipmap      = GL_FALSE
    };

    idk::glTextureConfig velocity_config = {
        .internalformat = GL_RGBA16F,
        .format         = GL_RGBA,
        .minfilter      = GL_LINEAR,
        .magfilter      = GL_LINEAR,
        .datatype       = GL_FLOAT,
        .genmipmap      = GL_FALSE
    };

    idk::glTextureConfig depth_config = {
        .internalformat = GL_DEPTH_COMPONENT24,
        .format         = GL_RED,
        .minfilter      = GL_NEAREST,
        .magfilter      = GL_NEAREST,
        .datatype       = GL_FLOAT
    };

    idk::glTextureConfig depth2_config =  {
        .internalformat = GL_R8,
        .format         = GL_RED
    };

    // static const idk::DepthAttachmentConfig depth_config = {
    //     .internalformat = GL_DEPTH_COMPONENT32F,
    //     .datatype       = GL_FLOAT
    // };

    m_dirshadow_buffer.reset(2048, 2048, 1);
    m_dirshadow_buffer.depthArrayAttachment(5, depth_config);

    m_dirshadow2_buffer.reset(2048, 2048, 4);
    m_dirshadow2_buffer.colorAttachment(0, depth2_config);
    m_dirshadow2_buffer.colorAttachment(1, depth2_config);
    m_dirshadow2_buffer.colorAttachment(2, depth2_config);
    m_dirshadow2_buffer.colorAttachment(3, depth2_config);

    // m_shadow_buffer.reset(w/1, h/1, 1);
    // m_shadow_buffer.colorAttachment(0, shadow_config);


    int div = m_rendersettings->volumetrics.res_divisor;
    m_volumetrics_buffer.reset(w/div, h/div, 1);
    m_volumetrics_buffer.colorAttachment(0, vol_config);
    // m_volumetrics_buffers[1].reset(w/2, h/2, 1);
    // m_volumetrics_buffers[1].colorAttachment(0, vol_config);

    // m_foliage_buffer.reset(w, h, 2);
    // m_foliage_buffer.colorAttachment(0, foliage_config);
    // m_foliage_buffer.depthAttachment(1, depth_config);


    m_SSAO_buffers[0].reset(w/1, h/1, 1);
    m_SSAO_buffers[1].reset(w/1, h/1, 1);
    m_SSAO_buffers[0].colorAttachment(0, SSAO_config);
    m_SSAO_buffers[1].colorAttachment(0, SSAO_config);

    m_finalbuffer.reset(m_winsize.x, m_winsize.y, 1);
    m_finalbuffer.colorAttachment(0, config);

    for (int i=0; i<3; i++)
    {
        m_lightbuffers[i] = new idk::glFramebuffer;
        m_lightbuffers[i]->reset(w, h, 1);
        m_lightbuffers[i]->colorAttachment(0, config);
    }


    m_mipbuffer[0].reset(w/2, h/2, 1);
    m_mipbuffer[1].reset(w/2, h/2, 6);
    m_mipbuffer[0].colorAttachment(0, SSR_config);
    for (int i=0; i<6; i++)
    {
        m_mipbuffer[1].colorAttachment(i, SSR_config);
    }


    m_ui_buffer.reset(m_winsize.x, m_winsize.y, 2);
    m_ui_buffer.colorAttachment(0, config);
    m_ui_buffer.depthAttachment(1, depth_config);


    idk::glTextureConfig albedo_config = {
        .internalformat = GL_RGBA16F,
        .format         = GL_RGBA,
        .minfilter      = GL_LINEAR,
        .magfilter      = GL_LINEAR,
        .datatype       = GL_FLOAT,
        .genmipmap      = GL_FALSE
    };

    idk::glTextureConfig normal_config = {
        .internalformat = GL_RGB_SNORM,
        .format         = GL_RGB,
        .minfilter      = GL_LINEAR,
        .magfilter      = GL_LINEAR,
        .datatype       = GL_UNSIGNED_BYTE,
        .genmipmap      = GL_FALSE
    };

    idk::glTextureConfig pbr_config = {
        .internalformat = GL_RGBA8,
        .format         = GL_RGBA,
        .minfilter      = GL_LINEAR,
        .magfilter      = GL_LINEAR,
        .datatype       = GL_UNSIGNED_BYTE,
        .genmipmap      = GL_FALSE
    };


    for (int i=0; i<2; i++)
    {
        m_gbuffers[i] = new idk::glFramebuffer;
        m_gbuffers[i]->reset(w, h, 5);
        m_gbuffers[i]->colorAttachment(0, albedo_config);
        m_gbuffers[i]->colorAttachment(1, normal_config);
        m_gbuffers[i]->colorAttachment(2, pbr_config);
        m_gbuffers[i]->colorAttachment(3, velocity_config);
        m_gbuffers[i]->depthAttachment(4, depth_config);

        // for (int j=0; j<4; j++)
        // {
        //     uint32_t texture = m_gbuffers[i]->attachments[j];
        //     uint64_t handle  = gl::getTextureHandleARB(texture);            
        //     m_gbuffers[i]->handles.push_back(handle);
        // }
    }

    m_temp_depthbuffer.reset(w, h, 1);
    m_temp_depthbuffer.depthAttachment(0, depth_config);

    // for (int i=0; i<4; i++)
    // {
    //     gl::makeTextureHandleResidentARB(m_gbuffers[1]->handles[i]);
    // }


    config = {
        .internalformat = GL_RGBA16F,
        .format         = GL_RGBA,
        .minfilter      = GL_LINEAR,
        .magfilter      = GL_LINEAR,
        .datatype       = GL_FLOAT,
        .genmipmap      = GL_FALSE
    };

    for (int i=0; i<=BLOOM_MAX_LEVEL; i++)
    {
        m_bloom_buffers[i].reset(w/pow(2, i), h/pow(2, i), 1);
        m_bloom_buffers[i].colorAttachment(0, config);
    }

}


void
idk::RenderEngine::_gen_envprobes()
{
    // idk::glTextureConfig envprobe_config = {
    //     .target         = GL_TEXTURE_CUBE_MAP,
    //     .internalformat = GL_RGBA8,
    //     .format         = GL_RGBA,
    //     .minfilter      = GL_LINEAR,
    //     .magfilter      = GL_LINEAR,
    //     .datatype       = GL_UNSIGNED_BYTE,
    //     .genmipmap      = GL_FALSE,
    // };

    // idk::DepthAttachmentConfig envprobe_depthconfig = {
    //     .internalformat = GL_DEPTH_COMPONENT16,
    //     .format         = GL_DEPTH_COMPONENT,
    //     .datatype       = GL_FLOAT,
    //     .compare_func   = GL_LESS,
    // };

    // idk::glTextureConfig lightprobe_config = {
    //     .target         = GL_TEXTURE_CUBE_MAP_ARRAY,
    //     .internalformat = GL_RGBA8,
    //     .format         = GL_RGBA,
    //     .minfilter      = GL_LINEAR,
    //     .magfilter      = GL_LINEAR,
    //     .datatype       = GL_UNSIGNED_BYTE,
    //     .genmipmap      = GL_FALSE,
    // };

    // m_envprobe_buffer.reset(256, 256, 1);
    // m_envprobe_buffer.cubeColorAttachment(0, envprobe_config);
    // m_envprobe_buffer.cubeDepthAttachment(envprobe_depthconfig);

    // int nprobes = m_rendersettings->envprobe.nprobes;
    // idk_printvalue(nprobes);

    // m_lightprobe_buffer.reset(16, 16, 1);
    // m_lightprobe_buffer.cubeArrayColorAttachment(0, nprobes, lightprobe_config);

}



void
idk::RenderEngine::init_all( std::string name, int w, int h )
{
    compileShaders();
    gl::enable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    init_screenquad();
    init_framebuffers(w, h);

    m_RQ           = m_render_queues.create();
    m_particle_RQ  = m_render_queues.create();
    m_decal_RQ     = m_render_queues.create();
    m_GI_RQ        = m_render_queues.create();
    m_shadow_RQ    = m_render_queues.create();

    // Primitive shapes
    // -----------------------------------------------------------------------------------------
    m_unit_cube        = modelAllocator().loadModel("IDKGE/resources/unit-cube.idkvi");
    m_unit_sphere      = modelAllocator().loadModel("IDKGE/resources/unit-sphere.idkvi");
    m_unit_sphere_FF   = modelAllocator().loadModel("IDKGE/resources/unit-sphere-FF.idkvi");
    m_unit_line        = modelAllocator().loadModel("IDKGE/resources/unit-line.idkvi");
    m_unit_cone        = modelAllocator().loadModel("IDKGE/resources/unit-cone.idkvi");
    m_unit_cylinder_FF = modelAllocator().loadModel("IDKGE/resources/unit-cylinder-FF.idkvi");
    // -----------------------------------------------------------------------------------------


    // Initialize buffers for indirect draw
    // -----------------------------------------------------------------------------------------
    m_UBO.init(0);
    m_UBO.bufferData(sizeof(idk::UBO_Buffer),  nullptr, GL_DYNAMIC_DRAW);

    m_SSBO.init(0);
    m_SSBO.bufferData(sizeof(idk::SSBO_Buffer), nullptr, GL_DYNAMIC_DRAW);

    m_DIB.init();
    m_DIB.bufferData(storage_buffer::MAX_DRAW_CALLS*sizeof(glDrawCmd), nullptr, GL_DYNAMIC_DRAW);

    m_lightsource_DIB.init();
    m_lightsource_DIB.bufferData(sizeof(glDrawCmd), nullptr, GL_DYNAMIC_DRAW);
    // -----------------------------------------------------------------------------------------


    // Compute BRDF LUT
    // -----------------------------------------------------------------------------------------
    idk::glTextureConfig config = {
        .internalformat = GL_RG16F,
        .format         = GL_RG,
        .minfilter      = GL_LINEAR,
        .magfilter      = GL_LINEAR,
        .wrap_s         = GL_CLAMP_TO_EDGE,
        .wrap_t         = GL_CLAMP_TO_EDGE,
        .datatype       = GL_FLOAT,
        .genmipmap      = GL_FALSE
    };

    static constexpr size_t LUT_TEXTURE_SIZE = 512;

    BRDF_LUT = idk::gltools::loadTexture2D(LUT_TEXTURE_SIZE, LUT_TEXTURE_SIZE, nullptr, config);
    idk::gl::bindImageTexture(0, BRDF_LUT, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG16F);

    idk::glShaderProgram program("IDKGE/shaders/brdf-lut.comp");
    program.bind();
    program.dispatch(LUT_TEXTURE_SIZE/8, LUT_TEXTURE_SIZE/8, 1);
    idk::gl::memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    // -----------------------------------------------------------------------------------------

    // _gen_envprobes();

    idk::internal::upload_noise();

    m_active_camera_id = createCamera();
    loadSkybox("IDKGE/resources/skybox/");
}



int
idk::RenderEngine::createProgram( const std::string &name, const idk::glShaderProgram &program )
{
    int id = m_programs.create(program);
    m_program_ids[name] = id;
    return id;
}


int
idk::RenderEngine::createProgram( const std::string &name, const std::string &root,
                                  const std::string &vs, const std::string &fs )
{
    auto VS = idk::glShaderStage((root+vs).c_str());
    auto FS = idk::glShaderStage((root+fs).c_str());
    return createProgram(name, idk::glShaderProgram(VS, FS));
}


int
idk::RenderEngine::createFramebuffer( float wscale, float hscale, int attachments )
{
    int id = m_framebuffers.create();

    auto &fb = m_framebuffers.get(id);
          fb.init(int(wscale*width()), int(hscale*height()), attachments);

    return id;
}


idk::Framebuffer&
idk::RenderEngine::getFramebuffer( int fb )
{
    return m_framebuffers.get(fb);
}




int
idk::RenderEngine::createRenderQueue( const std::string &program )
{
    for (auto &queue: m_user_render_queues)
    {
        if (queue.name == program)
        {
            LOG_WARN() << "[RenderEngine] Render queue already exists: \"" << program << "\"\n";
            return queue.ID;
        }
    }

    int ID = m_user_render_queues.create();
    LOG_INFO() << "[RenderEngine] Created user-facing render queue \"" << program << "\" with ID " << ID;

    m_user_render_queues.get(ID).config = { .cull_face = true };
    m_user_render_queues.get(ID).name   = program;
    m_user_render_queues.get(ID).ID     = ID;

    return ID;
}


int
idk::RenderEngine::createRenderQueue( const std::string &program, const idk::RenderQueueConfig &config )
{
    for (auto &queue: m_user_render_queues)
    {
        if (queue.name == program)
        {
            LOG_WARN() << "[RenderEngine] Render queue already exists: \"" << program << "\"\n";
            return queue.ID;
        }
    }

    int ID = m_user_render_queues.create();
    LOG_INFO() << "[RenderEngine] Created user-facing render queue \"" << program << "\" with ID " << ID;

    m_user_render_queues.get(ID).config = config;
    m_user_render_queues.get(ID).name   = program;
    m_user_render_queues.get(ID).ID     = ID;

    return ID;
}


void
idk::RenderEngine::destroyRenderQueue( int RQ )
{
    m_user_render_queues.destroy(RQ);
}


int
idk::RenderEngine::createShadowCasterQueue( const std::string &program )
{
    int ID = m_user_shadow_queues.create();
    LOG_INFO() << "Created user-facing shadow caster queue \"" << program << "\" with ID " << ID;

    m_user_shadow_queues.get(ID).name = program;
    return ID;
}


void
idk::RenderEngine::destroyShadowCasterQueue( int RQ )
{
    m_user_shadow_queues.destroy(RQ);
}



int
idk::RenderEngine::loadSkybox( const std::string &filepath )
{
    static const std::vector<std::string> faces
    {
        "px.png", "nx.png", "py.png", "ny.png", "pz.png", "nz.png"
    };

    static const std::vector<std::string> faces2
    {
        "0px.png", "0nx.png", "0py.png", "0ny.png", "0pz.png", "0nz.png"
    };

    static const glTextureConfig skybox_config = {
        .internalformat = GL_SRGB8_ALPHA8,
        .format         = GL_RGBA,
        .minfilter      = GL_LINEAR,
        .magfilter      = GL_LINEAR,
        .datatype       = GL_UNSIGNED_BYTE,
        .genmipmap      = GL_FALSE
    };

    static const glTextureConfig diffuse_config = {
        .internalformat = GL_RGBA8,
        .format         = GL_RGBA,
        .minfilter      = GL_LINEAR,
        .magfilter      = GL_LINEAR,
        .datatype       = GL_UNSIGNED_BYTE,
        .genmipmap      = GL_FALSE
    };


    static const glTextureConfig specular_config = {
        .internalformat = GL_RGBA8,
        .format         = GL_RGBA,
        .minfilter      = GL_LINEAR_MIPMAP_LINEAR,
        .magfilter      = GL_LINEAR,
        .datatype       = GL_UNSIGNED_BYTE,
        .genmipmap      = GL_FALSE,
        .setmipmap      = GL_TRUE,
        .texbaselevel   = 0,
        .texmaxlevel    = 5
    };

    GLuint skybox   = gltools::loadCubemap(filepath, faces, skybox_config);
    GLuint diffuse  = gltools::loadCubemap(filepath + "diffuse/", faces, diffuse_config);
    GLuint specular = gltools::loadCubemap(filepath + "specular/", faces2, specular_config);

    for (GLint mip=0; mip<=5; mip++)
    {
        gltools::loadCubemapMip(filepath + "specular/", faces, specular_config, specular, mip);
    }

    skyboxes.push_back(skybox);
    skyboxes_IBL.push_back(std::make_pair(diffuse, specular));

    LOG_INFO() << "Loaded skybox \"" << filepath << "\"";

    return skyboxes.size() - 1;
}



void
idk::RenderEngine::f_fbfb( glShaderProgram &program, glFramebuffer &in )
{
    gl::bindFramebuffer(GL_FRAMEBUFFER, 0);

    for (size_t i=0; i < in.attachments.size(); i++)
        program.set_sampler2D("un_texture_" + std::to_string(i), in.attachments[i]);

    gl::drawArrays(GL_TRIANGLES, 0, 6);
}



void
idk::RenderEngine::tex2tex( glShaderProgram &program, glFramebuffer &in, glFramebuffer &out )
{
    out.bind();

    for (size_t i=0; i < in.attachments.size(); i++)
    {
        program.set_sampler2D("un_texture_" + std::to_string(i), in.attachments[i]);
    }

    gl::drawArrays(GL_TRIANGLES, 0, 6);
};



void
idk::RenderEngine::tex2tex( glShaderProgram &program, glFramebuffer &a, glFramebuffer &b, glFramebuffer &out )
{
    out.bind();

    size_t textureID = 0;
    for (size_t i=0; i < a.attachments.size(); i++)
    {
        program.set_sampler2D("un_texture_" + std::to_string(i), a.attachments[i]);
    }

    for (size_t i=0; i < b.attachments.size(); i++)
    {
        program.set_sampler2D("un_texture_" + std::to_string(4+i), b.attachments[i]);
    }

    gl::drawArrays(GL_TRIANGLES, 0, 6);
};


void
idk::RenderEngine::setRenderSetting( idk::RenderSetting flag, bool value )
{
    if (value)
    {
        m_render_settings |= static_cast<uint32_t>(flag);
    }

    else
    {
        m_render_settings &= ~static_cast<uint32_t>(flag);
    }
}


bool
idk::RenderEngine::getRenderSetting( idk::RenderSetting flag )
{
    return m_render_settings & static_cast<uint32_t>(flag);
}




int
idk::RenderEngine::createCamera()
{
    int camera_id = m_cameras.create();
    m_cameras.get(camera_id).aspect = float(m_resolution.x) / float(m_resolution.y);
    return camera_id;
}


void
idk::RenderEngine::drawSphere( const glm::vec3 position, float radius )
{
    glm::mat4 T = glm::translate(glm::mat4(1.0f), position);
    glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(radius));

    drawModel(m_unit_sphere_FF, T*S);
}


void
idk::RenderEngine::drawSphere( const glm::mat4 &M )
{
    drawModel(m_unit_sphere_FF, M);
}


void
idk::RenderEngine::drawRect( const glm::mat4 &M )
{
    drawModel(m_unit_cube, M);
}




void
idk::RenderEngine::drawLine( const glm::vec3 A, const glm::vec3 B, float thickness )
{
    glm::mat4 T = glm::translate(glm::mat4(1.0f), A);
    glm::mat4 R = glm::transpose(glm::lookAt(A, B, glm::vec3(0.0f, 1.0f, 0.0f)));
              R = glm::mat4_cast(glm::normalize(glm::quat_cast(R)));

    glm::mat4 S  = glm::scale(glm::mat4(1.0f), glm::vec3(thickness, thickness, glm::length(B - A)));

    drawModel(m_unit_cylinder_FF, T*R*S);
}


void
idk::RenderEngine::drawCapsule( const glm::vec3 top, const glm::vec3 bottom, float thickness )
{
    drawSphere(top, thickness);
    drawSphere(bottom, thickness);
    drawLine(top, bottom, 2.0f*thickness);
}


int
idk::RenderEngine::loadModel( const std::string &filepath )
{
    return m_model_allocator.loadModel(filepath);
}


int
idk::RenderEngine::loadModelLOD( int model, int level, const std::string &filepath )
{
    return m_model_allocator.loadModelLOD(model, level, filepath);
}


void
idk::RenderEngine::drawModel( int model, const glm::mat4 &transform )
{
    _getRenderQueue(m_RQ).enque(model, transform);
}

void
idk::RenderEngine::drawModel( int model, const glm::mat4 &transform, const glm::mat4 &prev )
{
    _getRenderQueue(m_RQ).enque(model, transform, prev);
}


void
idk::RenderEngine::drawModel( int model, const idk::Transform &T )
{
    _getRenderQueue(m_RQ).enque(model, T, getCamera(), modelAllocator());
}


void
idk::RenderEngine::drawModelRQ( int RQ, int model, const glm::mat4 &transform )
{
    m_user_render_queues.get(RQ).enque(model, transform);
}


void
idk::RenderEngine::drawShadowCasterRQ( int RQ, int model, const glm::mat4 &transform )
{
    m_user_shadow_queues.get(RQ).enque(model, transform);
}


void
idk::RenderEngine::drawDecal( int model, const glm::vec3 &pos, const glm::vec3 &dir, float scale )
{
    glm::mat4 M = glm::inverse(glm::lookAt(pos, pos-dir, glm::vec3(0.0f, 1.0f, 0.0f)));
    glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(scale));

    _getRenderQueue(m_decal_RQ).enque(model, M*S);
}


void
idk::RenderEngine::drawTextureOverlay( uint32_t texture )
{
    m_texture_overlays.push(texture);
}



void
idk::RenderEngine::drawShadowCaster( int model, const glm::mat4 &transform )
{
    _getRenderQueue(m_shadow_RQ).enque(model, transform);
}


void
idk::RenderEngine::drawEnvironmental( int model, const glm::mat4 &transform )
{
    _getRenderQueue(m_GI_RQ).enque(model, transform);
}


void
idk::RenderEngine::pushRenderOverlay( const std::string &filepath,
                                      float fadein, float display, float fadeout )
{
    static const idk::glTextureConfig config = {
        .internalformat = GL_SRGB8_ALPHA8,
        .format         = GL_RGBA,
        .minfilter      = GL_LINEAR,
        .magfilter      = GL_LINEAR,
        .wrap_s         = GL_CLAMP_TO_BORDER,
        .wrap_t         = GL_CLAMP_TO_BORDER,
        .datatype       = GL_UNSIGNED_BYTE,
        .genmipmap      = false
    };

    idk::TextureRef texture = gltools::loadTextureWrapper(filepath, config);
    m_overlays.push(idk::RenderOverlay(texture, fadein, display, fadeout));
}


void
idk::RenderEngine::pushRenderOverlayFill( const glm::vec3 &fill,
                                          float fadein, float display, float fadeout )
{
    m_overlayfills.push(RenderOverlayFill(fill, fadein, display, fadeout));
}


void
idk::RenderEngine::pushRenderOverlay( const RenderOverlay &overlay )
{
    m_overlays.push(overlay);
}



void
idk::RenderEngine::pushRenderOverlayFill( const RenderOverlayFill &overlay )
{
    m_overlayfills.push(overlay);
}


void
idk::RenderEngine::skipRenderOverlay()
{
    if (m_overlays.empty() == false)
    {
        m_overlays.pop();
    }

    else if (m_overlayfills.empty() == false)
    {
        m_overlayfills.pop();
    }
}


void
idk::RenderEngine::skipAllRenderOverlays()
{
    while (m_overlays.empty() == false)
    {
        m_overlays.pop();
    }
}



void
idk::RenderEngine::update_UBO_camera( idk::UBO_Buffer &buffer )
{
    uint32_t offset = 0;

    for (IDK_Camera &camera: m_cameras)
    {
        camera.width    = m_resolution.x;
        camera.height   = m_resolution.y;
        camera.aspect   = camera.width / camera.height;
        camera.position = glm::inverse(camera.V)[3];

        camera.P = glm::perspective(
            glm::radians(camera.fov - camera.fov_offset),
            camera.aspect,
            camera.near,
            camera.far
        );

        glm::vec2 jitter = halton_sequence[idk::getFrame() % 128];
                  jitter = jitter * 2.0f - 1.0f;
                  jitter *= m_rendersettings->taa.scale;
                  jitter /= glm::vec2(camera.width, camera.height);

        camera.prev_jitter = camera.jitter;
        camera.jitter      = glm::vec4(jitter, 0.0f, 0.0f);


        buffer.cameras[offset] = camera;

        glm::mat4 J = glm::mat4(1.0f);
        J[3][0] += jitter.x;
        J[3][1] += jitter.y;

        IDK_Camera &bcam = buffer.cameras[offset];

        bcam.P          = J * camera.P;
        bcam.P_nojitter = camera.P;

        camera.prev_P          = bcam.P;
        camera.prev_P_nojitter = bcam.P_nojitter;

        offset += 1;

        if (offset >= m_cameras.size())
        {
            break;
        }
    }
}



idk::glDrawCmd
idk::RenderEngine::genLightsourceDrawCommand( int model, uint32_t num_lights, idk::ModelAllocator &MA )
{
    MeshDescriptor &mesh = MA.getModel(model).meshes[0];

    idk::glDrawCmd cmd = {
        .count         = mesh.numIndices,
        .instanceCount = num_lights,
        .firstIndex    = mesh.firstIndex,
        .baseVertex    = mesh.baseVertex,
        .baseInstance  = 0
    };

    return cmd;
}


void
idk::RenderEngine::update_UBO_lightsources( idk::UBO_Buffer &buffer )
{
    static uint32_t offset;

    IDK_Camera &camera = getCamera();


    offset = 0;
    for (IDK_Dirlight &light: m_dirlights)
    {
        auto matrices = glDepthCascade::computeCascadeMatrices(
            camera.fov, camera.aspect, camera.near, camera.far,
            2048.0f,
            camera.V, glm::vec3(light.direction), light.cascades, light.cascade_zmult
        );

        // light.cascades = glm::vec4(16.0f,  32.0f,  128.0f, 512.0f);

        glm::mat4 P = glm::ortho(-35.0f, +35.0f, -35.0f, +35.0f, -35.0f, +35.0f);
        glm::mat4 V = glm::lookAt(
            glm::vec3(0.0f),
            glm::vec3(light.direction),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );


        light.transform = P*V;

        for (int i=0; i<4; i++)
        {
            light.transforms[i] = matrices[i];
        }

        buffer.dirlights[offset++] = light;
    }


    offset = 0;
    for (IDK_Pointlight &light: m_pointlights)
    {
        // glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(light.position));
        // glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f*light.radius));

        // light.transform = T * S;

        buffer.pointlights[offset++] = light;
    }

    offset = 0;
    for (IDK_Spotlight &light: m_spotlights)
    {
        // float h      = light.radius;
        // float alpha  = light.angle[0];
        // float radius = h * tan(alpha);

        // glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(light.position));
        // glm::mat4 R = glm::mat4_cast(light.orientation);
        // glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f*radius));

        // light.transform = T * R * S;
        // light.direction = light.orientation * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);

        buffer.spotlights[offset++] = light;
    }
}


static bool memsame( const void *a, const void *b, uint32_t nbytes )
{
    uint8_t *buf0 = (uint8_t *)(a);
    uint8_t *buf1 = (uint8_t *)(b);

    for (uint32_t i=0; i<nbytes; i++)
    {
        if (buf0[i] != buf1[i])
        {
            return false;
        }
    }

    return true;
}



void
idk::RenderEngine::applyRenderSettings( const RenderSettings &settings, bool refresh )
{
    // RenderConfig &rconfig = *m_config;

    // if (rconfig.changed == false)
    // {
    //     return;
    // }
    // rconfig.changed = false;


    // for (auto &[group_name, group]: rconfig.groups)
    // {
    //     if (group.changed == false)
    //     {
    //         continue;
    //     }
    //     group.changed = false;


    //     // auto &program = getBindProgram(group.program);

    //     // for (auto &[field_name, field]: group)
    //     // {
    //     //     program.set_float("un_"+field_name, field.getValue());
    //     // }
    // }



    // auto prev = *m_rendersettings;
    // auto curr = settings;

    if (refresh == false)
    {
        if (memsame(m_rendersettings, &settings, sizeof(RenderSettings)) == true)
        {
            return;
        }
    }

    LOG_INFO() << "Render settings changed";



    // TAA
    {
        auto prev = m_rendersettings->taa;
        auto curr = settings.taa;

        // if (memsame(prev, curr) == false)
        {
            // LOG_INFO() << "TAA settings changed";

            auto &program = getBindProgram("TAA");
            program.set_float("un_factor", float(settings.taa.factor));
        }

        // m_rendersettings->taa = settings.taa;
    }


    // SSAO
    {
        // auto prev = m_rendersettings->ssao;
        // auto curr = settings.ssao;

        // m_rendersettings->ssao = curr;
    }

    // Volumetrics
    {
        auto prev = m_rendersettings->volumetrics;
        auto curr = settings.volumetrics;

        // if (memsame(prev, curr) == false)
        {
            // LOG_INFO() << "Volumetrics settings changed";

            idk::glTextureConfig vol_config = {
                .internalformat = GL_RGBA8,
                .format         = GL_RGBA,
                .minfilter      = GL_LINEAR,
                .magfilter      = GL_LINEAR,
                .datatype       = GL_UNSIGNED_BYTE,
                .genmipmap      = GL_FALSE
            };

            // int div = m_rendersettings->volumetrics.res_divisor;
            // m_volumetrics_buffer.reset(m_resolution.x/div, m_resolution.y/div, 1);
            // m_volumetrics_buffer.colorAttachment(0, vol_config);

            auto &program = getBindProgram("deferred-dirlight");
            // program.set_int("un_samples",          settings.volumetrics.samples);
            // program.set_int("un_samples_sun",      settings.volumetrics.samples_sun);
            program.set_float("un_intensity",      settings.volumetrics.intensity);
            program.set_float("un_height_offset",  settings.volumetrics.height_offset);
            program.set_float("un_height_falloff", settings.volumetrics.height_falloff);
            // program.set_float("un_scatter_coeff",  settings.volumetrics.scatter_coeff);
            // program.set_float("un_absorb_coeff",   settings.volumetrics.absorb_coeff);
            // program.set_float("un_worley_amp",     settings.volumetrics.worley_amp);
            // program.set_float("un_worley_wav",     settings.volumetrics.worley_wav);
        }


        // m_rendersettings->volumetrics = settings.volumetrics;
    }

    // // Environment probes
    // {
    //     auto prev = m_rendersettings->envprobe;
    //     auto curr = settings.envprobe;

    //     if (curr.grid_size != prev.grid_size)
    //     {
    //         curr.dirty = true;
    //     }
    
    //     if (curr.cell_size != prev.cell_size)
    //     {
    //         curr.dirty = true;
    //     }

    //     m_rendersettings->envprobe = curr;
    // }

    *m_rendersettings = settings;

}


const idk::RenderSettings &
idk::RenderEngine::getRenderSettings()
{
    return *m_rendersettings;
}



void
idk::RenderEngine::beginFrame()
{
    gl::bindFramebuffer(GL_FRAMEBUFFER, 0);
    gl::viewport(0, 0, m_resolution.x, m_resolution.y);
    gl::clearColor(0.0f, 0.0f, 0.0f, 1.0f);
    gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_should_recompile)
    {
        _recompileShaders();
        applyRenderSettings(*m_rendersettings, true);
        m_should_recompile = false;
    }

    idk::updateTime(delta_time);
}


void
idk::RenderEngine::endFrame( float dt )
{
    delta_time = dt;
    IDK_Camera &camera = getCamera();


    // Swap G Buffers
    // {
    //     gl::bindFramebuffer(GL_FRAMEBUFFER, 0);

    //     static idk::glBufferObject<GL_UNIFORM_BUFFER> ubo_gbuffer(
    //         shader_bindings::UBO_GBuffer,
    //         8 * sizeof(uint64_t),
    //         GL_DYNAMIC_COPY
    //     );

    //     for (int i=0; i<4; i++)
    //     {
    //         gl::makeTextureHandleResidentARB(m_gbuffers[0]->handles[i]);
    //         gl::makeTextureHandleNonResidentARB(m_gbuffers[1]->handles[i]);
    //     }

    //     ubo_gbuffer.bufferSubData(0, 4*sizeof(uint64_t), m_gbuffers[0]->attachments.data());

    //     std::swap(m_gbuffers[0], m_gbuffers[1]);
    // }



    {
        if (m_overlays.empty() == false)
        {
            m_overlays.front().advance(dt);

            if (m_overlays.front().finished())
            {
                LOG_INFO() << "Overlay texture " << m_overlays.front().texture.ID() << " finished";
                m_overlays.pop();
            }
        }


        if (m_overlayfills.empty() == false)
        {
            m_overlayfills.front().advance(dt);

            if (m_overlayfills.front().finished())
            {
                m_overlayfills.pop();
            }
        }
    }


    // Update particle emitters
    // -----------------------------------------------------------------------------------------
    idk::ParticleSystem::update(*this, dt, camera.position, _getRenderQueue(m_particle_RQ));
    // -----------------------------------------------------------------------------------------

    // Update terrain rendering
    // -----------------------------------------------------------------------------------------
    idk::TerrainRenderer::update(*this, camera, m_model_allocator);
    // -----------------------------------------------------------------------------------------


    // Update UBO data
    // -----------------------------------------------------------------------------------------
    m_UBO_buffer.counter = (m_UBO_buffer.counter + 1) % 4;

    update_UBO_camera(m_UBO_buffer);
    update_UBO_lightsources(m_UBO_buffer);
    // -----------------------------------------------------------------------------------------


    // Update SSBO data
    // -----------------------------------------------------------------------------------------
    {
        auto &MA = m_model_allocator;

        size_t texture_offset   = 0;
        size_t transform_offset = 0;
        size_t drawID_offset    = 0;

        for (auto &queue: m_render_queues)
        {
            queue.genDrawCommands(
                MA,
                texture_offset,
                transform_offset,
                drawID_offset,
                m_SSBO_buffer,
                m_DIB_buffer
            );
        }

        for (auto &queue: m_user_render_queues)
        {
            queue.genDrawCommands(
                MA,
                texture_offset,
                transform_offset,
                drawID_offset,
                m_SSBO_buffer,
                m_DIB_buffer
            );
        }

        for (auto &queue: m_user_shadow_queues)
        {
            queue.genDrawCommands(
                MA,
                texture_offset,
                transform_offset,
                drawID_offset,
                m_SSBO_buffer,
                m_DIB_buffer
            );
        }
    }
    // -----------------------------------------------------------------------------------------


    IDK_ASSERT("m_DIB_buffer.size() > 1024", m_DIB_buffer.size() <= storage_buffer::MAX_DRAW_CALLS);

    m_UBO.bufferSubData(0, sizeof(m_UBO_buffer), (const void *)(&m_UBO_buffer));
    m_SSBO.bufferSubData(0, sizeof(m_SSBO_buffer), (const void *)(&m_SSBO_buffer));
    m_DIB.bufferSubData(0, sizeof(idk::glDrawCmd)*m_DIB_buffer.size(), m_DIB_buffer.data());
    gl::bindBuffer(GL_DRAW_INDIRECT_BUFFER, m_DIB.ID());


    gl::disable(GL_BLEND);
    gl::enable(GL_DEPTH_TEST, GL_CULL_FACE);
    gl::enable(GL_TEXTURE_CUBE_MAP_SEAMLESS);


    // Perform environment mapping
    // -----------------------------------------------------------------------------------------
    gl::bindVertexArray(m_model_allocator.getVAO());
    RenderStage_envmapping(camera, dt);
    // -----------------------------------------------------------------------------------------


    // Render G-Buffer
    // -----------------------------------------------------------------------------------------
    gl::bindVertexArray(m_model_allocator.getVAO());

    std::swap(m_gbuffers[0], m_gbuffers[1]);
    m_gbuffers[0]->bind();
    m_gbuffers[0]->clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    RenderStage_geometry(camera, dt, *(m_gbuffers[0]));
    // -----------------------------------------------------------------------------------------


    // Render depth maps
    // -----------------------------------------------------------------------------------------
    gl::bindVertexArray(m_model_allocator.getVAO());
    gl::disable(GL_CULL_FACE);
    gl::enable(GL_POLYGON_OFFSET_FILL);
    IDK_GLCALL( glPolygonOffset(1.0f, 1.0f); )
    // gl::cullFace(GL_FRONT);
    shadowpass_dirlights();
    shadowpass_pointlights();
    shadowpass_spotlights();
    // gl::cullFace(GL_BACK);
    gl::enable(GL_CULL_FACE);

    {
        auto &program = getBindProgram("shadowcascade-split");
        gl::bindImageTextures(0, 4, m_dirshadow2_buffer.attachments.data());

        program.set_sampler2DArray("un_input", m_dirshadow_buffer.depth_attachment);

        program.set_int("un_image_w", 2048);
        program.set_int("un_image_h", 2048);
        program.dispatch(2048/8, 2048/8, 1);
        gl::memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }
    // -----------------------------------------------------------------------------------------


    // Foliage
    // -----------------------------------------------------------------------------------------
    // gl::bindVertexArray(m_model_allocator.getVAO());

    // m_foliage_buffer.bind();
    // m_foliage_buffer.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // idk::TerrainRenderer::renderGrass(*this, m_foliage_buffer, camera, m_model_allocator);
    // -----------------------------------------------------------------------------------------


    // Lighting pass
    // -----------------------------------------------------------------------------------------
    std::swap(m_lightbuffers[0], m_lightbuffers[1]);
    m_lightbuffers[0]->bind();
    m_lightbuffers[0]->clear(GL_COLOR_BUFFER_BIT);

    RenderStage_lighting(camera, dt, *(m_gbuffers[0]), *(m_lightbuffers[0]));
    // -----------------------------------------------------------------------------------------


    // Custom post processing render stages
    // -----------------------------------------------------------------------------------------
    // for (auto *RS: m_render_stages)
    // {
    //     RS->update(*this, getFramebuffer(m_postprocess_fb));
    // }
    // -----------------------------------------------------------------------------------------


    // Post processing pass
    // -----------------------------------------------------------------------------------------
    gl::disable(GL_DEPTH_TEST, GL_CULL_FACE);
    RenderStage_postprocessing(camera, *(m_lightbuffers[0]), m_finalbuffer);
    // -----------------------------------------------------------------------------------------


    // Clear render queues
    // -----------------------------------------------------------------------------------------
    for (idk::RenderQueue &queue: m_render_queues)
    {
        queue.clear();
    }

    for (idk::RenderQueue &queue: m_user_render_queues)
    {
        queue.clear();
    }

    for (idk::RenderQueue &queue: m_user_shadow_queues)
    {
        queue.clear();
    }

    m_DIB_buffer.clear();
    // -----------------------------------------------------------------------------------------

}


void
idk::RenderEngine::swapWindow()
{
    SDL_GL_SwapWindow(this->getWindow());
}



void
idk::RenderEngine::setRenderScale( float scale )
{
    m_renderscale = scale;
    resize(m_winsize.x, m_winsize.y);
}



void
idk::RenderEngine::resize( int w, int h )
{
    w = 8 * (w / 8);
    h = 8 * (h / 8);


    m_winsize = glm::ivec2(w, h);
    m_resolution = glm::ivec2(glm::vec2(m_winsize) * m_renderscale);

    // m_resolution.x = w;  m_resolution.y = h;
    init_framebuffers(m_resolution.x, m_resolution.y);
    getCamera().aspect = float(w) / float(h);

    // for (auto &[id, fb]: m_framebuffers)
    // {
    //     fb.resize(w, h);
    // }

    SDL_SetWindowSize(getWindow(), w, h);

}










static void
idk_temp_VXGI_code()
{
    // static bool vxgi_on = getRenderSetting(RenderSetting::VXGI);

    // if (vxgi_on != getRenderSetting(RenderSetting::VXGI))
    // {
    //     auto &program = getProgram("lpass");
        
    //     if (vxgi_on)    program.setDefinition("VXGI_ON", "0");
    //     else            program.setDefinition("VXGI_ON", "1");

    //     program.compile();
    // }



    // vxgi_on = getRenderSetting(RenderSetting::VXGI);

    // if (getRenderSetting(RenderSetting::VXGI))
    // {
    //     gl::bindImageTextures(0, 6, &vxgi_radiance_2[0]);
    //     gl::bindImageTextures(8, 6, &vxgi_radiance[0]);
    //     gl::bindImageTexture(6, vxgi_albedo, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8UI);
    //     gl::bindImageTexture(7, vxgi_normal, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8UI);


    //     if (getRenderSetting(RenderSetting::VXGI_LIVE_VOXELIZATION))
    //     {
    //         // Clear volume textures
    //         // -----------------------------------------------------------------------------------------
    //         {
    //             auto &program = getProgram("vxgi-clear");
    //             program.bind();
    //             program.dispatch(VXGI_TEXTURE_SIZE / 4);
    //             gl::memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    //         }
    //         // -----------------------------------------------------------------------------------------


    //         VXGI::shadowPass(
    //             m_vxgi_buffer,
    //             camera,
    //             lightSystem().getDirlight(0).direction,
    //             getProgram("vxgi-shadow"),
    //             modelAllocator().getDrawIndirectBuffer(),
    //             commands
    //         );


    //         VXGI::renderTexture(
    //             m_vxgi_buffer,
    //             camera,
    //             getProgram("vxgi-voxelize"),
    //             modelAllocator().getDrawIndirectBuffer(),
    //             commands,
    //             m_lightsystem.depthCascade()
    //         );

    //     }

    //     static int vxgi_face   = 0;
    //     static int vxgi_offset = 0;

    //     vxgi_face = (vxgi_face + 1) % 6;
    //     vxgi_offset = (vxgi_offset + 1) % 4;

    //     if (getRenderSetting(RenderSetting::VXGI_LIVE_INJECTION))
    //     {
    //         {
    //             auto &program = getProgram("vxgi-inject");
    //             program.bind();
    //             program.set_int("un_face", vxgi_face);
    //             program.set_int("un_offset", vxgi_offset);

    //             VXGI::injectRadiance(
    //                 program,
    //                 camera,
    //                 m_vxgi_buffer,
    //                 lightSystem().getDirlight(0).direction,
    //                 lightSystem().depthCascade()
    //             );
    //         }
    //         // VXGI::generateMipmap(getProgram("vxgi-mipmap-1"), getProgram("vxgi-mipmap-2"), vxgi_radiance_2);

    //         // {
    //         //     auto &program = getProgram("vxgi-propagate");
    //         //     program.bind();
    //         //     program.set_int("un_face", vxgi_face);
    //         //     program.set_int("un_offset", vxgi_offset);

    //         //     for (int i=0; i<6; i++)
    //         //     {
    //         //         program.set_sampler2D("un_input_radiance[" + std::to_string(i) + "]", vxgi_radiance_2[i]);
    //         //     }
    //         //     program.dispatch(VXGI_TEXTURE_SIZE/4);
    //         //     gl::memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    //         // }
    //         VXGI::generateMipmap(getProgram("vxgi-mipmap-1"), getProgram("vxgi-mipmap-2"), vxgi_radiance);
    //     }
    // }

}




