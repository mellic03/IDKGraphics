#include "idk_renderengine.hpp"
#include "idk_render_settings.hpp"

#include "render/idk_vxgi.hpp"
#include "render/cubemap.hpp"

#include "particle/particle_system.hpp"
#include "terrain/terrain.hpp"

#include <libidk/idk_noisegen.hpp>



void
idk::RenderEngine::RenderStage_envmapping( IDK_Camera &camera, float dtime )
{
    // static uint32_t layer = 0;
    // static uint32_t face  = 0;

    // face = (face + 1) % 6;

    // if (face == 0)
    // {
    //     layer = (layer + 1) % (m_rendersettings->envprobe.nprobes);
    // }

    // gl::disable(GL_CULL_FACE);
    // gl::enable(GL_DEPTH_TEST);

    // {
    //     m_envprobe_buffer.bind();
    //     auto &queue = _getRenderQueue(m_GI_RQ);

    //     glm::vec3 position = env_probe::layerToWorld(
    //         camera.position, layer, m_rendersettings->envprobe
    //     );

    //     _render_envmap(
    //         position,
    //         face,
    //         queue,
    //         m_envprobe_buffer
    //     );

    // }

    // gl::disable(GL_DEPTH_TEST);

    // {
    //     m_lightprobe_buffer.bind();

    //     auto &program = getProgram("convolve-cubemap");
    //     program.bind();

    //     _convolve_cubemap(
    //         m_envprobe_buffer.attachments[0],
    //         layer,
    //         face,
    //         program,
    //         m_lightprobe_buffer
    //     );
    // }

    // gl::enable(GL_CULL_FACE, GL_DEPTH_TEST);

}



static void drawRQ( idk::glShaderProgram &program, idk::RenderQueue &RQ )
{
    using namespace idk;

    program.set_uint("un_draw_offset", uint32_t(RQ.getDrawCommandOffset()));

    gl::multiDrawElementsIndirect(
        GL_TRIANGLES,
        GL_UNSIGNED_INT,
        (const void *)(sizeof(idk::glDrawCmd) * RQ.getDrawCommandOffset()),
        RQ.numDrawCommands(),
        sizeof(idk::glDrawCmd)
    );
}



void
idk::RenderEngine::RenderStage_geometry( IDK_Camera &camera, float dtime,
                                         glFramebuffer &buffer_out )
{
    buffer_out.bind();
    gl::bindVertexArray(modelAllocator().getVAO());

    // Internal geometry pass render queue
    // -----------------------------------------------------------------------------------------
    {
        idk::RenderQueue &queue = _getRenderQueue(m_RQ);

        if (queue.numDrawCommands() > 0)
        {
            auto &program = getBindProgram("gpass");
            drawRQ(program, queue);
        }
    }
    // -----------------------------------------------------------------------------------------


    // User-created render queues
    // -----------------------------------------------------------------------------------------
    for (idk::RenderQueue &queue: m_user_render_queues)
    {
        if (queue.numDrawCommands() == 0)
        {
            continue;
        }

        auto &program = getBindProgram(queue.name);

        if (queue.config.cull_face == false)
        {
            gl::disable(GL_CULL_FACE);
            drawRQ(program, queue);
            gl::enable(GL_CULL_FACE);
        }

        else
        {
            drawRQ(program, queue);
        }
    }


    // Decals
    // -----------------------------------------------------------------------------------------
    {
        idk::RenderQueue &queue = _getRenderQueue(m_decal_RQ);

        if (queue.numDrawCommands() > 0)
        {
            auto &program = getBindProgram("gpass-decal");
            drawRQ(program, queue);
        }
    }
    // -----------------------------------------------------------------------------------------


    // terrain
    // -----------------------------------------------------------------------------------------
    gl::disable(GL_BLEND);
    {
        buffer_out.bind();

        idk::TerrainRenderer::render(*this, buffer_out, dtime, camera, m_model_allocator);

        gl::bindVertexArray(m_model_allocator.getVAO());
        gl::bindBuffer(GL_DRAW_INDIRECT_BUFFER, m_DIB.ID());
    }
    // -----------------------------------------------------------------------------------------


    // Particles
    // -----------------------------------------------------------------------------------------
    {
        buffer_out.bind();

        idk::ParticleSystem::render(m_model_allocator);

        gl::bindVertexArray(m_model_allocator.getVAO());
        gl::bindBuffer(GL_DRAW_INDIRECT_BUFFER, m_DIB.ID());
    }
    // -----------------------------------------------------------------------------------------

}



// void
// idk::RenderEngine::RenderStage_atmospheres( IDK_Camera   &camera,
//                                             glFramebuffer &buffer_in,
//                                             glFramebuffer &buffer_out )
// {
//     idk::glDrawCmd cmd = genAtmosphereDrawCommand(modelAllocator());
//     if (cmd.instanceCount == 0)
//     {
//         return;
//     }
//     m_DrawCommandBuffer.bufferSubData(0, sizeof(idk::glDrawCmd), &cmd);

//     auto &program = getProgram("atmosphere");
//     program.bind();

//     program.set_sampler2D("un_texture_0", m_gbuffers[0]->attachments[0]);
//     program.set_sampler2D("un_texture_1", m_gbuffers[0]->attachments[1]);
//     program.set_sampler2D("un_texture_2", m_gbuffers[0]->attachments[2]);

//     program.set_sampler2D("un_fragdepth", m_gbuffers[0]->depth_attachment);
//     program.set_sampler2D("un_BRDF_LUT", BRDF_LUT);

//     gl::multiDrawElementsIndirect(
//         GL_TRIANGLES,
//         GL_UNSIGNED_INT,
//         nullptr,
//         1,
//         sizeof(idk::glDrawCmd)
//     );

// }


void
idk::RenderEngine::RenderStage_volumetrics()
{
    idk::glDrawCmd cmd = genLightsourceDrawCommand(
        m_unit_cube, m_dirlights.size(), modelAllocator()
    );

    if (cmd.instanceCount == 0)
    {
        return;
    }

    m_lightsource_DIB.bufferSubData(0, sizeof(idk::glDrawCmd), &cmd);
    gl::bindBuffer(GL_DRAW_INDIRECT_BUFFER, m_lightsource_DIB.ID());


    static double irrational = 1.61803398875;
    irrational += 1.61803398875;

    m_volumetrics_buffers[0]->bind();
    m_volumetrics_buffers[0]->clear(GL_COLOR_BUFFER_BIT);


    auto &config = m_rendersettings->volumetrics;

    auto &program = getProgram("dirlight-volumetric");
    program.bind();

    program.set_float("un_samples",     config.samples);
    program.set_float("un_attenuation", config.attenuation);
    program.set_float("un_intensity",   config.intensity);
    program.set_float("un_factor",      config.factor);

    // // program.set_sampler2D("un_previous",  m_volumetrics_buffers[(idx+1)%2].attachments[0]);
    program.set_float("un_irrational",    fmod(irrational, 1.0));

    program.set_sampler2D("un_fragdepth", m_gbuffers[0]->attachments[3]);
    program.set_sampler2D("un_shadowmap", m_dirshadow_buffer.depth_attachment);

    gl::multiDrawElementsIndirect(
        GL_TRIANGLES,
        GL_UNSIGNED_INT,
        nullptr,
        1,
        sizeof(idk::glDrawCmd)
    );
}


void
idk::RenderEngine::RenderStage_radiance()
{
    // auto &program = getProgram("radiance");
    // program.bind();


    // {
    //     uint32_t texture = m_lightprobe_buffer.attachments[0];
    
    //     program.set_ivec3("un_probe_grid_size", m_rendersettings->envprobe.grid_size);
    //     program.set_vec3("un_probe_cell_size",  m_rendersettings->envprobe.cell_size);
    //     program.set_samplerCube("IDK_UN_PROBE_ARRAY", texture);
    // }

    // program.set_sampler2D("un_texture_0", m_gbuffers[0]->attachments[0]);
    // program.set_sampler2D("un_texture_1", m_gbuffers[0]->attachments[1]);
    // program.set_sampler2D("un_texture_2", m_gbuffers[0]->attachments[2]);
    // program.set_sampler2D("un_fragdepth", m_gbuffers[0]->depth_attachment);
    // program.set_sampler2D("un_BRDF_LUT",  BRDF_LUT);

    // program.set_samplerCube("un_skybox_diffuse", skyboxes_IBL[current_skybox].first);
    // program.set_samplerCube("un_skybox_specular", skyboxes_IBL[current_skybox].second);

    // gl::drawArrays(GL_TRIANGLES, 0, 6);

}


void
idk::RenderEngine::RenderStage_dirlights()
{
    idk::glDrawCmd cmd = genLightsourceDrawCommand(
        m_unit_cube, m_dirlights.size(), modelAllocator()
    );

    if (cmd.instanceCount == 0)
    {
        return;
    }

    m_lightsource_DIB.bufferSubData(0, sizeof(idk::glDrawCmd), &cmd);
    gl::bindBuffer(GL_DRAW_INDIRECT_BUFFER, m_lightsource_DIB.ID());

    auto &program = getProgram("deferred-dirlight");
    program.bind();

    program.set_sampler2D("un_texture_0", m_gbuffers[0]->attachments[0]);
    program.set_sampler2D("un_texture_1", m_gbuffers[0]->attachments[1]);
    program.set_sampler2D("un_texture_2", m_gbuffers[0]->attachments[2]);
    program.set_sampler2D("un_fragdepth", m_gbuffers[0]->attachments[3]);
    program.set_sampler2D("un_BRDF_LUT",  BRDF_LUT);

    program.set_samplerCube("un_skybox", skyboxes[current_skybox]);
    program.set_samplerCube("un_skybox_diffuse", skyboxes_IBL[current_skybox].first);
    program.set_samplerCube("un_skybox_specular", skyboxes_IBL[current_skybox].second);
    program.set_sampler2DArray("un_shadowmap", m_dirshadow_buffer.depth_attachment);

    gl::multiDrawElementsIndirect(
        GL_TRIANGLES,
        GL_UNSIGNED_INT,
        nullptr,
        1,
        sizeof(idk::glDrawCmd)
    );
}


void
idk::RenderEngine::RenderStage_pointlights()
{
    idk::glDrawCmd cmd = genLightsourceDrawCommand(
        m_unit_sphere, m_pointlights.size(), modelAllocator()
    );

    if (cmd.instanceCount == 0)
    {
        return;
    }

    m_lightsource_DIB.bufferSubData(0, sizeof(idk::glDrawCmd), &cmd);
    gl::bindBuffer(GL_DRAW_INDIRECT_BUFFER, m_lightsource_DIB.ID());

    auto &program = getProgram("deferred-pointlight");
    program.bind();

    program.set_sampler2D("un_texture_0", m_gbuffers[0]->attachments[0]);
    program.set_sampler2D("un_texture_1", m_gbuffers[0]->attachments[1]);
    program.set_sampler2D("un_texture_2", m_gbuffers[0]->attachments[2]);
    program.set_sampler2D("un_fragdepth", m_gbuffers[0]->attachments[3]);
    program.set_sampler2D("un_BRDF_LUT",  BRDF_LUT);

    gl::multiDrawElementsIndirect(
        GL_TRIANGLES,
        GL_UNSIGNED_INT,
        nullptr,
        1,
        sizeof(idk::glDrawCmd)
    );
}


void
idk::RenderEngine::RenderStage_spotlights()
{
    idk::glDrawCmd cmd = genLightsourceDrawCommand(
        m_unit_sphere, m_spotlights.size(), modelAllocator()
    );

    if (cmd.instanceCount == 0)
    {
        return;
    }

    m_lightsource_DIB.bufferSubData(0, sizeof(idk::glDrawCmd), &cmd);
    gl::bindBuffer(GL_DRAW_INDIRECT_BUFFER, m_lightsource_DIB.ID());

    auto &program = getProgram("deferred-spotlight");
    program.bind();

    program.set_sampler2D("un_texture_0", m_gbuffers[0]->attachments[0]);
    program.set_sampler2D("un_texture_1", m_gbuffers[0]->attachments[1]);
    program.set_sampler2D("un_texture_2", m_gbuffers[0]->attachments[2]);
    program.set_sampler2D("un_fragdepth", m_gbuffers[0]->attachments[3]);
    program.set_sampler2D("un_BRDF_LUT",  BRDF_LUT);

    gl::multiDrawElementsIndirect(
        GL_TRIANGLES,
        GL_UNSIGNED_INT,
        nullptr,
        1,
        sizeof(idk::glDrawCmd)
    );
}



void
idk::RenderEngine::RenderStage_lighting( IDK_Camera &camera, float dtime,
                                         glFramebuffer &buffer_in,
                                         glFramebuffer &buffer_out )
{
    gl::disable(GL_BLEND);
    // gl::disable(GL_BLEND, GL_CULL_FACE);
    // m_radiance_buffer.bind();
    // RenderStage_radiance();

    // SSAO
    // -----------------------------------------------------------------------------------------
    static int SSAO_curr = 0;
    static int SSAO_next = 1;
    SSAO_curr = (SSAO_curr + 1) % 2;
    SSAO_next = (SSAO_curr + 1) % 2;

    if (m_rendersettings->ssao.enabled)
    {
        auto &config = m_rendersettings->ssao;

        static double irrational = 1.61803398875;
        irrational += 1.61803398875;
        // irrational = fmod(irrational, 1.61803398875);

        m_SSAO_buffers[SSAO_curr].bind();
        m_SSAO_buffers[SSAO_curr].clear(GL_COLOR_BUFFER_BIT);

        auto &program = getProgram("SSAO");

        program.bind();
        program.set_sampler2D("un_gNormal",   m_gbuffers[0]->attachments[1]);
        program.set_sampler2D("un_gDepth",    m_gbuffers[0]->attachments[3]);
        program.set_sampler2D("un_prev",      m_SSAO_buffers[SSAO_next].attachments[0]);

        program.set_float("un_irrational",    fmod(irrational, 1.0));
        program.set_float("un_intensity",     config.intensity);
        program.set_float("un_factor",        config.factor);
        program.set_int("un_samples",         config.samples);
        program.set_float("un_ssao_radius",   config.radius);
        program.set_float("un_ssao_bias",     config.bias);

        gl::drawArrays(GL_TRIANGLES, 0, 6);
    }
    // -----------------------------------------------------------------------------------------


    gl::bindVertexArray(m_model_allocator.getVAO());


    // Volumetrics
    // -----------------------------------------------------------------------------------------
    if (m_rendersettings->volumetrics.enabled)
    {
        RenderStage_volumetrics();
    }
    // -----------------------------------------------------------------------------------------




    buffer_out.bind();
    buffer_out.clear(GL_COLOR_BUFFER_BIT);

    gl::enable(GL_BLEND, GL_CULL_FACE);
    gl::blendFunc(GL_ONE, GL_ONE);
    gl::cullFace(GL_FRONT);

    RenderStage_dirlights();
    RenderStage_pointlights();
    RenderStage_spotlights();


    gl::bindVertexArray(m_quad_VAO);
    gl::disable(GL_CULL_FACE, GL_DEPTH_TEST);


    if (m_rendersettings->ssao.enabled)
    {
        gl::blendFunc(GL_DST_COLOR, GL_ZERO);

        auto &program = getProgram("SSAO_composite");
        program.bind();
        program.set_sampler2D("un_input", m_SSAO_buffers[SSAO_curr].attachments[0]);
        gl::drawArrays(GL_TRIANGLES, 0, 6);

        gl::blendFunc(GL_SRC_ALPHA, GL_ONE);
    }



    if (m_rendersettings->volumetrics.enabled)
    {
        gl::blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        auto &program = getProgram("blit");
        program.bind();
        program.set_sampler2D("un_input", m_volumetrics_buffers[0]->attachments[0]);
        gl::drawArrays(GL_TRIANGLES, 0, 6);

        gl::blendFunc(GL_SRC_ALPHA, GL_ONE);
    }


    // if (m_rendersettings->volumetrics.enabled)
    // {
    //     gl::blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //     glShaderProgram &program = getProgram("composite");
    //     program.bind();
    //     program.set_sampler2D("un_input", m_volumetrics_buffers[0].attachments[0]);
    //     gl::drawArrays(GL_TRIANGLES, 0, 6);

    //     gl::blendFunc(GL_SRC_ALPHA, GL_ONE);
    // }


    gl::enable(GL_CULL_FACE);
    gl::cullFace(GL_BACK);

}


void
idk::RenderEngine::PostProcess_chromatic_aberration( glFramebuffer &buffer_in,
                                                     glFramebuffer &buffer_out )
{
    glShaderProgram &program = getProgram("chromatic");
    program.bind();

    program.set_sampler2D("un_input", buffer_in.attachments[0]);

    buffer_out.bind();
    gl::drawArrays(GL_TRIANGLES, 0, 6);
}


void
idk::RenderEngine::PostProcess_SSR( glFramebuffer &buffer_in, glFramebuffer &buffer_out )
{
    // glBlitNamedFramebuffer(
    //     buffer_in.m_FBO,
    //     m_mip_scratchbuffer.m_FBO,
    //     0, 0, m_mip_scratchbuffer.size().x, m_mip_scratchbuffer.size().y,
    //     0, 0, width(), height(),
    //     GL_COLOR_BUFFER_BIT,
    //     GL_LINEAR
    // );

    // gl::enable(GL_BLEND);
    // gl::blendFunc(GL_SRC_ALPHA, GL_ONE);

    // gl::generateTextureMipmap(m_mip_scratchbuffer.attachments[0]);

    buffer_out.bind();
    buffer_out.clear(GL_COLOR_BUFFER_BIT);


    auto &program = getProgram("SSR");
    program.bind();

    program.set_sampler2D("un_input",  buffer_in.attachments[0]);
    program.set_sampler2D("un_albedo", m_gbuffers[0]->attachments[0]);
    program.set_sampler2D("un_normal", m_gbuffers[0]->attachments[1]);
    program.set_sampler2D("un_pbr",    m_gbuffers[0]->attachments[2]);
    program.set_sampler2D("un_fragdepth", m_gbuffers[0]->attachments[3]);
    program.set_sampler2D("un_BRDF_LUT",  BRDF_LUT);
    program.set_samplerCube("un_skybox",  skyboxes[current_skybox]);

    gl::drawArrays(GL_TRIANGLES, 0, 6);

    // gl::disable(GL_BLEND);
}


void
idk::RenderEngine::PostProcess_bloom( glFramebuffer &buffer_in )
{
    for (int i=0; i<=BLOOM_MAX_LEVEL; i++)
    {
        m_bloom_buffers[i].clear(GL_COLOR_BUFFER_BIT);
    }


    {
        glShaderProgram &program = getProgram("bloom-first");
        program.bind();
        tex2tex(program, buffer_in, m_bloom_buffers[0]);
    }



    {
        glShaderProgram &program = getProgram("bloom-down");
        program.bind();

        for (int level=0; level<BLOOM_MAX_LEVEL; level++)
        {
            tex2tex(program, m_bloom_buffers[level], m_bloom_buffers[level+1]);
        }
    }


    gl::enable(GL_BLEND);
    gl::blendFunc(GL_ONE, GL_ONE);

    {
        glShaderProgram &program = getProgram("bloom-up");
        program.bind();

        for (int level=BLOOM_MAX_LEVEL; level>0; level--)
        {
            program.set_float("un_miplevel", float(level));
            tex2tex(program, m_bloom_buffers[level], m_bloom_buffers[level-1]);
        }
    }

    gl::disable(GL_BLEND);
}


void
idk::RenderEngine::PostProcess_colorgrading( IDK_Camera &camera,
                                             glFramebuffer &buffer_in,
                                             glFramebuffer &buffer_out )
{
    buffer_out.bind();

    glShaderProgram &program = getProgram("colorgrade");
    program.bind();
    program.set_sampler2D("un_input", buffer_in.attachments[0]);
    program.set_sampler2D("un_bloom", m_bloom_buffers[0].attachments[0]);

    gl::drawArrays(GL_TRIANGLES, 0, 6);
}


void
idk::RenderEngine::PostProcess_text( glFramebuffer &buffer_out )
{
    static const idk::glTextureConfig config = {
        .internalformat = GL_RGBA16,
        .format         = GL_RGBA,
        .minfilter      = GL_LINEAR,
        .magfilter      = GL_LINEAR,
        .datatype       = GL_UNSIGNED_BYTE,
        .genmipmap      = GL_FALSE
    };

    GLuint texture = gltools::loadTexture2D(width(), height(), m_textsurface->pixels, config);

    gl::bindImageTexture(0, buffer_out.attachments[0], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);

    glShaderProgram &program = getProgram("text");
    program.bind();

    program.set_sampler2D("un_text", texture);
    program.dispatch(width()/8, height()/8, 1);

    gl::deleteTextures(1, &texture);
}


void
idk::RenderEngine::PostProcess_ui( glFramebuffer &buffer_out )
{
    buffer_out.bind();

    gl::enable(GL_BLEND);
    gl::blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glShaderProgram &program = getProgram("button-rect");
    program.bind();

    program.set_sampler2D("un_input", m_ui_buffer.attachments[0]);
    gl::drawArrays(GL_TRIANGLES, 0, 6);

    gl::disable(GL_BLEND);

}



void
idk::RenderEngine::PostProcess_overlay( idk::glFramebuffer &buffer_out )
{
    gl::bindImageTexture(0, buffer_out.attachments[0], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);

    if (m_overlayfills.empty() == false)
    {
        RenderOverlayFill &overlay = m_overlayfills.front();

        glShaderProgram &program = getProgram("overlay-fill");
        program.bind();

        program.set_float ("un_alpha", overlay.alpha);
        program.set_vec3  ("un_color", overlay.fill);

        program.dispatch(width()/8, height()/8, 1);
    }

    if (m_overlays.empty() == false)
    {
        RenderOverlay &overlay = m_overlays.front();

        glShaderProgram &program = getProgram("overlay");
        program.bind();

        program.set_float     ("un_alpha", overlay.alpha);
        program.set_sampler2D ("un_input", overlay.texture.ID());

        program.dispatch(width()/8, height()/8, 1);
    }
}


void
idk::RenderEngine::RenderStage_postprocessing( IDK_Camera   &camera,
                                               glFramebuffer &buffer_in,
                                               glFramebuffer &buffer_out )
{
    // idk::glFramebuffer *src = &m_scratchbuffers2[0];
    // idk::glFramebuffer *dst = &buffer_out;

    // PostProcess_SSR(buffer_in, m_scratchbuffers2[0]);
    PostProcess_bloom(buffer_in);
    // std::swap(src, dst);

    // // PostProcess_chromatic_aberration(*A, *B);
    // // std::swap(src, dst);

    PostProcess_colorgrading(camera, buffer_in, m_scratchbuffers2[1]);
    // // std::swap(src, dst);


    buffer_out.bind();

    // FXAA
    // -----------------------------------------------------------------------------------------
    {
        auto &program = getProgram("fxaa");
        program.bind();
        program.set_sampler2D("un_input", m_scratchbuffers2[1].attachments[0]);

        gl::drawArrays(GL_TRIANGLES, 0, 6);
    }
    // -----------------------------------------------------------------------------------------

    PostProcess_text(buffer_out);
    gl::memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    PostProcess_ui(buffer_out);
    gl::memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    PostProcess_overlay(buffer_out);
    gl::memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    {
        gl::bindFramebuffer(GL_FRAMEBUFFER, 0);

        IDK_GLCALL(
            glBlitNamedFramebuffer(
                buffer_out.m_FBO,
                0,
                0, 0, width(), height(),
                0, 0, width(), height(),
                GL_COLOR_BUFFER_BIT,
                GL_NEAREST
            );
        )
    }


    gl::enable(GL_DEPTH_TEST, GL_CULL_FACE);
}
