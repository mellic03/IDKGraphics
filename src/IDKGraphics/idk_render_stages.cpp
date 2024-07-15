#include "idk_renderengine.hpp"
#include "idk_render_settings.hpp"

#include "render/idk_vxgi.hpp"

#include <libidk/idk_noisegen.hpp>
#include "render/cubemap.hpp"

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
            auto &program = getProgram("gpass");
            program.bind();
            program.set_uint("un_draw_offset", uint32_t(queue.getDrawCommandOffset()));

            gl::multiDrawElementsIndirect(
                GL_TRIANGLES,
                GL_UNSIGNED_INT,
                (const void *)(sizeof(idk::glDrawCmd) * queue.getDrawCommandOffset()),
                queue.numDrawCommands(),
                sizeof(idk::glDrawCmd)
            );
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

        if (queue.config.cull_face == false)
        {
            gl::disable(GL_CULL_FACE);
        }

        auto &program = getProgram(queue.name);
        program.bind();
        program.set_uint("un_draw_offset", uint32_t(queue.getDrawCommandOffset()));

        gl::multiDrawElementsIndirect(
            GL_TRIANGLES,
            GL_UNSIGNED_INT,
            (const void *)(sizeof(idk::glDrawCmd) * queue.getDrawCommandOffset()),
            queue.numDrawCommands(),
            sizeof(idk::glDrawCmd)
        );

        if (queue.config.cull_face == false)
        {
            gl::enable(GL_CULL_FACE);
        }
    }
    // -----------------------------------------------------------------------------------------


    // Particles
    // -----------------------------------------------------------------------------------------
    {
        idk::RenderQueue &queue = _getRenderQueue(m_viewspace_RQ);

        if (queue.numDrawCommands() > 0)
        {
            auto &program = getProgram("gpass-particle");
            program.bind();
            program.set_uint("un_draw_offset", uint32_t(queue.getDrawCommandOffset()));

            gl::multiDrawElementsIndirect(
                GL_TRIANGLES,
                GL_UNSIGNED_INT,
                (const void *)(sizeof(idk::glDrawCmd) * queue.getDrawCommandOffset()),
                queue.numDrawCommands(),
                sizeof(idk::glDrawCmd)
            );
        }
    }
    // -----------------------------------------------------------------------------------------


    // Decals
    // -----------------------------------------------------------------------------------------
    {
        idk::RenderQueue &queue = _getRenderQueue(m_decal_RQ);

        if (queue.numDrawCommands() > 0)
        {
            auto &program = getProgram("gpass-decal");
            program.bind();
            program.set_uint("un_draw_offset", uint32_t(queue.getDrawCommandOffset()));

            gl::multiDrawElementsIndirect(
                GL_TRIANGLES,
                GL_UNSIGNED_INT,
                (const void *)(sizeof(idk::glDrawCmd) * queue.getDrawCommandOffset()),
                queue.numDrawCommands(),
                sizeof(idk::glDrawCmd)
            );
        }
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

//     program.set_sampler2D("un_texture_0", m_gbuffer.attachments[0]);
//     program.set_sampler2D("un_texture_1", m_gbuffer.attachments[1]);
//     program.set_sampler2D("un_texture_2", m_gbuffer.attachments[2]);

//     program.set_sampler2D("un_fragdepth", m_gbuffer.depth_attachment);
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
idk::RenderEngine::RenderStage_volumetrics( int idx )
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


    static double irrational = 0.25;
    irrational += 0.0025 * 1.61803398875;
    irrational = fmod(irrational, 1.0);

    m_volumetrics_buffers[idx].bind();
    m_volumetrics_buffers[idx].clear(GL_COLOR_BUFFER_BIT);

    auto &program = getProgram("dirlight-volumetric");
    program.bind();

    program.set_sampler2D("un_previous",  m_volumetrics_buffers[(idx+1)%2].attachments[0]);
    program.set_sampler2D("un_noise",     m_SSAO_noise);
    program.set_float("un_irrational",    irrational);

    program.set_sampler2D("un_fragdepth", m_gbuffer.depth_attachment);
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

    // program.set_sampler2D("un_texture_0", m_gbuffer.attachments[0]);
    // program.set_sampler2D("un_texture_1", m_gbuffer.attachments[1]);
    // program.set_sampler2D("un_texture_2", m_gbuffer.attachments[2]);
    // program.set_sampler2D("un_fragdepth", m_gbuffer.depth_attachment);
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

    program.set_sampler2D("un_texture_0", m_gbuffer.attachments[0]);
    program.set_sampler2D("un_texture_1", m_gbuffer.attachments[1]);
    program.set_sampler2D("un_texture_2", m_gbuffer.attachments[2]);
    program.set_sampler2D("un_fragdepth", m_gbuffer.depth_attachment);
    program.set_sampler2D("un_BRDF_LUT",  BRDF_LUT);

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

    program.set_sampler2D("un_texture_0", m_gbuffer.attachments[0]);
    program.set_sampler2D("un_texture_1", m_gbuffer.attachments[1]);
    program.set_sampler2D("un_texture_2", m_gbuffer.attachments[2]);
    program.set_sampler2D("un_fragdepth", m_gbuffer.depth_attachment);
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

    program.set_sampler2D("un_texture_0", m_gbuffer.attachments[0]);
    program.set_sampler2D("un_texture_1", m_gbuffer.attachments[1]);
    program.set_sampler2D("un_texture_2", m_gbuffer.attachments[2]);
    program.set_sampler2D("un_fragdepth", m_gbuffer.depth_attachment);
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
    gl::disable(GL_BLEND, GL_CULL_FACE);
    m_radiance_buffer.bind();
    RenderStage_radiance();

    // SSAO
    // -----------------------------------------------------------------------------------------
    static int SSAO_curr = 0;
    static int SSAO_next = 1;
    SSAO_curr = (SSAO_curr + 1) % 2;
    SSAO_next = (SSAO_curr + 1) % 2;

    if (SSAO_settings.enabled)
    {
        static double irrational = 1.0;
        irrational += 0.0025 * 1.61803398875;

        // while (irrational > 1.0)
        // {
        //     irrational -= 1.0;
        // }

        m_SSAO_buffers[SSAO_curr].bind();
        m_SSAO_buffers[SSAO_curr].clear(GL_COLOR_BUFFER_BIT);

        auto &program = getProgram("SSAO");

        program.bind();
        program.set_sampler2D("un_gNormal",   m_gbuffer.attachments[1]);
        program.set_sampler2D("un_gDepth",    m_gbuffer.depth_attachment);
        program.set_sampler2D("un_prev",      m_SSAO_buffers[SSAO_next].attachments[0]);
        program.set_sampler2D("un_noise",     m_SSAO_noise);

        program.set_float("un_irrational",    irrational);
        program.set_float("un_intensity",     SSAO_settings.intensity);
        program.set_float("un_factor",        SSAO_settings.factor);
        program.set_int("un_samples",         SSAO_settings.samples);
        program.set_float("un_ssao_radius",   SSAO_settings.radius);
        program.set_float("un_ssao_bias",     SSAO_settings.bias);

        gl::drawArrays(GL_TRIANGLES, 0, 6);
    }
    // -----------------------------------------------------------------------------------------


    gl::enable(GL_BLEND);
    gl::blendFunc(GL_SRC_ALPHA, GL_ONE);

    gl::enable(GL_CULL_FACE);
    gl::cullFace(GL_FRONT);

    m_scratchbuffers2[0].bind();
    m_scratchbuffers2[0].clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    gl::bindVertexArray(m_model_allocator.getVAO());

    RenderStage_dirlights();
    RenderStage_pointlights();
    RenderStage_spotlights();


    static int vol_curr = 0;
    static int vol_next = 1;
    vol_curr = (vol_curr + 1) % 2;
    vol_next = (vol_curr + 1) % 2;
    

    if (m_rendersettings->dirlight_volumetrics)
    {
        RenderStage_volumetrics(vol_curr);
        m_scratchbuffers2[0].bind();
    }


    gl::bindVertexArray(m_quad_VAO);
    gl::disable(GL_CULL_FACE, GL_DEPTH_TEST);

    {
        auto &program = getProgram("composite");
        program.bind();

        program.set_sampler2D("un_input", m_radiance_buffer.attachments[0]);
        gl::drawArrays(GL_TRIANGLES, 0, 6);
    }

    if (SSAO_settings.enabled)
    {
        gl::blendFunc(GL_DST_COLOR, GL_ZERO);

        auto &program = getProgram("SSAO_composite");
        program.bind();
        program.set_sampler2D("un_input", m_SSAO_buffers[SSAO_curr].attachments[0]);
        gl::drawArrays(GL_TRIANGLES, 0, 6);

        gl::blendFunc(GL_SRC_ALPHA, GL_ONE);
    }

    buffer_out.bind();

    // step alpha 0/1
    // -----------------------------------------------------------------------------------------
    {
        glShaderProgram &program = getProgram("alpha-0-1");
        program.bind();
        program.set_sampler2D("un_input", m_scratchbuffers2[0].attachments[0]);
        gl::drawArrays(GL_TRIANGLES, 0, 6);
    }
    // -----------------------------------------------------------------------------------------


    // Background
    // -----------------------------------------------------------------------------------------
    gl::blendFunc(GL_ONE_MINUS_DST_ALPHA, GL_DST_ALPHA);
    {
        glShaderProgram &program = getProgram("background");
        program.bind();
        program.set_samplerCube("un_skybox", skyboxes[current_skybox]);
        gl::drawArrays(GL_TRIANGLES, 0, 6);
    }
    // -----------------------------------------------------------------------------------------


    if (m_rendersettings->dirlight_volumetrics)
    {
        gl::blendFunc(GL_SRC_ALPHA, GL_ONE);

        glShaderProgram &program = getProgram("composite");
        program.bind();
        program.set_sampler2D("un_input", m_volumetrics_buffers[vol_curr].attachments[0]);
        gl::drawArrays(GL_TRIANGLES, 0, 6);
    }

    gl::disable(GL_DEPTH_TEST);

    gl::disable(GL_CULL_FACE);
    gl::cullFace(GL_BACK);

    // Volumetrics
    // -----------------------------------------------------------------------------------------

    gl::disable(GL_BLEND);
    gl::enable(GL_CULL_FACE);
    // // -----------------------------------------------------------------------------------------

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
    glBlitNamedFramebuffer(
        buffer_in.m_FBO,
        m_mip_scratchbuffer.m_FBO,
        0, 0, m_mip_scratchbuffer.size().x, m_mip_scratchbuffer.size().y,
        0, 0, width(), height(),
        GL_COLOR_BUFFER_BIT,
        GL_LINEAR
    );

    gl::enable(GL_BLEND);
    gl::blendFunc(GL_SRC_ALPHA, GL_ONE);

    gl::generateTextureMipmap(m_mip_scratchbuffer.attachments[0]);

    buffer_out.bind();

    auto &program = getProgram("SSR");
    program.bind();

    program.set_sampler2D("un_input",  m_mip_scratchbuffer.attachments[0]);
    program.set_sampler2D("un_albedo", m_gbuffer.attachments[0]);
    program.set_sampler2D("un_normal", m_gbuffer.attachments[1]);
    program.set_sampler2D("un_pbr",    m_gbuffer.attachments[2]);
    program.set_sampler2D("un_fragdepth", m_gbuffer.depth_attachment);
    program.set_sampler2D("un_BRDF_LUT",  BRDF_LUT);
    program.set_samplerCube("un_skybox",  skyboxes[current_skybox]);

    gl::drawArrays(GL_TRIANGLES, 0, 6);

    gl::disable(GL_BLEND);
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
    gl::memoryBarrier(GL_ALL_BARRIER_BITS);

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
        gl::memoryBarrier(GL_ALL_BARRIER_BITS);
    }

    if (m_overlays.empty() == false)
    {
        RenderOverlay &overlay = m_overlays.front();

        glShaderProgram &program = getProgram("overlay");
        program.bind();

        program.set_float     ("un_alpha", overlay.alpha);
        program.set_sampler2D ("un_input", overlay.texture.ID());

        program.dispatch(width()/8, height()/8, 1);
        gl::memoryBarrier(GL_ALL_BARRIER_BITS);
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
    PostProcess_ui(buffer_out);
    PostProcess_overlay(buffer_out);

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
