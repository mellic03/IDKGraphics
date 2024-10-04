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

        idk::TerrainRenderer::render(*this, dtime, buffer_out, m_temp_depthbuffer, camera, m_model_allocator);

        gl::bindVertexArray(m_model_allocator.getVAO());
        gl::bindBuffer(GL_DRAW_INDIRECT_BUFFER, m_DIB.ID());
    }
    // -----------------------------------------------------------------------------------------


    // Particles
    // -----------------------------------------------------------------------------------------
    {
        buffer_out.bind();

        idk::ParticleSystem::render(*this, m_model_allocator);

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


    m_volumetrics_buffer.bind();
    m_volumetrics_buffer.clear(GL_COLOR_BUFFER_BIT);

    // auto &config = m_rendersettings->volumetrics;

    auto &program = getProgram("dirlight-volumetric");
    program.bind();

    program.set_sampler2D("un_color",     m_lightbuffers[0]->attachments[0]);
    program.set_sampler2D("un_fragdepth", m_gbuffers[0]->attachments[4]);
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


    program.set_sampler2D("un_occlusion", m_SSAO_buffers[0].attachments[0]);

    program.set_sampler2D("un_texture_0", m_gbuffers[0]->attachments[0]);
    program.set_sampler2D("un_texture_1", m_gbuffers[0]->attachments[1]);
    program.set_sampler2D("un_texture_2", m_gbuffers[0]->attachments[2]);
    program.set_sampler2D("un_fragdepth", m_gbuffers[0]->attachments[4]);
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
    program.set_sampler2D("un_fragdepth", m_gbuffers[0]->attachments[4]);
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
    program.set_sampler2D("un_fragdepth", m_gbuffers[0]->attachments[4]);
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
    if (m_rendersettings->ssao.enabled)
    {
        auto &config = m_rendersettings->ssao;

        {
            m_SSAO_buffers[1].bind();
            m_SSAO_buffers[1].clear(GL_COLOR_BUFFER_BIT);

            auto &program = getBindProgram("SSAO");
            program.set_sampler2D("un_gNormal",   m_gbuffers[0]->attachments[1]);
            program.set_sampler2D("un_gDepth",    m_gbuffers[0]->attachments[4]);

            program.set_float("un_intensity",     config.intensity);
            program.set_float("un_factor",        config.factor);
            program.set_int("un_samples",         config.samples);
            program.set_float("un_ssao_radius",   config.radius);
            program.set_float("un_ssao_bias",     config.bias);

            gl::drawArrays(GL_TRIANGLES, 0, 6);
        }


        // Screen-space shadows
        // -------------------------------------------------------------------------------------
        // {
        //     gl::enable(GL_BLEND);
        //     gl::blendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR);

        //     // m_shadow_buffer.bind();
        //     // m_shadow_buffer.clear(GL_COLOR_BUFFER_BIT);

        //     auto &program = getBindProgram("SSS");
        //     program.set_sampler2D("un_normal",    m_gbuffers[0]->attachments[1]);
        //     program.set_sampler2D("un_fragdepth", m_gbuffers[0]->attachments[4]);

        //     gl::drawArrays(GL_TRIANGLES, 0, 6);
        // }
        // -------------------------------------------------------------------------------------


        {
            int G = glm::clamp(m_rendersettings->ssao.iterations, 0, 8);
            auto &gaussian = getBindProgram("filter-gaussian");

            for (int i=0; i<G; i++)
            {
                m_SSAO_buffers[0].bind();
                m_SSAO_buffers[0].clear(GL_COLOR_BUFFER_BIT);
                gaussian.set_sampler2D("un_input", m_SSAO_buffers[1].attachments[0]);
                gaussian.set_int("un_horizontal", 0);
                gl::drawArrays(GL_TRIANGLES, 0, 6);

                m_SSAO_buffers[1].bind();
                m_SSAO_buffers[1].clear(GL_COLOR_BUFFER_BIT);
                gaussian.set_sampler2D("un_input", m_SSAO_buffers[0].attachments[0]);
                gaussian.set_int("un_horizontal", 1);
                gl::drawArrays(GL_TRIANGLES, 0, 6);
            }

            if (m_rendersettings->ssao.unsharp > 0)
            {
                auto &unsharp  = getBindProgram("filter-unsharp");
                m_SSAO_buffers[0].bind();
                m_SSAO_buffers[0].clear(GL_COLOR_BUFFER_BIT);
                unsharp.set_sampler2D("un_input", m_SSAO_buffers[1].attachments[0]);
                gl::drawArrays(GL_TRIANGLES, 0, 6);
            }
        }
    }
    // -----------------------------------------------------------------------------------------


    gl::bindVertexArray(m_model_allocator.getVAO());


    buffer_out.bind();

    gl::enable(GL_BLEND, GL_CULL_FACE);
    gl::blendFunc(GL_ONE, GL_ONE);
    gl::cullFace(GL_FRONT);

    RenderStage_dirlights();
    RenderStage_pointlights();
    RenderStage_spotlights();


    gl::bindVertexArray(m_quad_VAO);
    gl::disable(GL_CULL_FACE, GL_DEPTH_TEST);


    // if (m_rendersettings->ssao.enabled)
    // {
    //     gl::blendFunc(GL_DST_COLOR, GL_ZERO);

    //     auto &program = getProgram("SSAO_composite");
    //     program.bind();
    //     program.set_sampler2D("un_input", m_SSAO_buffers[0].attachments[0]);
    //     gl::drawArrays(GL_TRIANGLES, 0, 6);

    //     gl::blendFunc(GL_SRC_ALPHA, GL_ONE);
    // }

    // Blit foliage
    // -----------------------------------------------------------------------------------------
    // {
    //     gl::enable(GL_BLEND);
    //     gl::blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //     auto &program = getBindProgram("blit-foliage");
    //     program.set_sampler2D("un_foliage_color", m_foliage_buffer.attachments[0]);
    //     program.set_sampler2D("un_foliage_depth", m_foliage_buffer.attachments[1]);
    //     program.set_sampler2D("un_gbuffer_depth", m_gbuffers[0]->attachments[4]);
    
    //     gl::drawArrays(GL_TRIANGLES, 0, 6);
    // }
    // -----------------------------------------------------------------------------------------


    // Volumetrics
    // -----------------------------------------------------------------------------------------
    if (m_rendersettings->volumetrics.enabled)
    {
        gl::disable(GL_BLEND);
        gl::bindVertexArray(m_model_allocator.getVAO());
        RenderStage_volumetrics();

        gl::enable(GL_BLEND);
        gl::bindVertexArray(m_quad_VAO);
        buffer_out.bind();

        if (m_rendersettings->volumetrics.blend_mode == 0)
        {
            gl::blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
    
        else if (m_rendersettings->volumetrics.blend_mode == 1)
        {
            gl::blendFunc(GL_SRC_ALPHA, GL_ONE);
        }
    
        else
        {
            gl::blendFunc(GL_ONE, GL_ONE);
        }

        auto &program = getProgram("blit-volumetrics");
        program.bind();
        program.set_sampler2D("un_input", m_volumetrics_buffer.attachments[0]);
        gl::drawArrays(GL_TRIANGLES, 0, 6);
    }
    // ----------------------------------------------------------------------------------------

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
idk::RenderEngine::PostProcess_SSR( glFramebuffer &buffer_out )
{
    // IDK_GLCALL(
    //     glBlitNamedFramebuffer(
    //         buffer_out.m_FBO,
    //         m_mipbuffer[0].m_FBO,
    //         0, 0, width(), height(),
    //         0, 0, width()/2, height()/2,
    //         GL_COLOR_BUFFER_BIT,
    //         GL_NEAREST
    //     );
    // )
    // gl::generateTextureMipmap(m_mipbuffer[0].attachments[0]);

    {
        auto &program = getBindProgram("SSR-downsample");
        auto size     = m_mipbuffer[0].size();

        for (int i=0; i<6; i++)
        {
            uint32_t src;

            if (i == 0)
            {
                src = buffer_out.attachments[0];
                program.set_int("un_scale", 2);
            }

            else
            {
                src = m_mipbuffer[1].attachments[i-1];
                program.set_int("un_scale", 1);
            }


            program.set_sampler2D("un_input", src);

            gl::bindImageTexture(
                1, m_mipbuffer[1].attachments[i], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F
            );

            program.set_int("un_miplevel", i);
            program.dispatch(size.x/8, size.y/8, 1);

            gl::memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        }
    }


    buffer_out.bind();
    gl::enable(GL_BLEND);

    if (m_rendersettings->ssr.blend_mode == 0)
    {
        gl::blendFunc(GL_SRC_ALPHA, GL_ONE);
    }

    else
    {
        gl::blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }


    {
        auto &program = getBindProgram("SSR");
    
        for (int i=0; i<6; i++)
        {
            std::string label = "un_input[" + std::to_string(i) + "]";
            program.set_sampler2D(label, m_mipbuffer[1].attachments[i]);
        }

        program.set_sampler2D("un_albedo",    m_gbuffers[0]->attachments[0]);
        program.set_sampler2D("un_normal",    m_gbuffers[0]->attachments[1]);
        program.set_sampler2D("un_pbr",       m_gbuffers[0]->attachments[2]);
        program.set_sampler2D("un_fragdepth", m_gbuffers[0]->attachments[4]);
        program.set_sampler2D("un_BRDF_LUT",  BRDF_LUT);
        program.set_sampler2D("un_occlusion", m_SSAO_buffers[0].attachments[0]);
        program.set_samplerCube("un_skybox",  skyboxes[current_skybox]);
        gl::drawArrays(GL_TRIANGLES, 0, 6);
    }

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

    if (m_rendersettings->ssr.enabled)
    {
        PostProcess_SSR(*(m_lightbuffers[0]));
    }


    m_lightbuffers[1]->bind();
    // m_lightbuffers[1]->clear(GL_COLOR_BUFFER_BIT);

    // TAA
    // -----------------------------------------------------------------------------------------
    {
        gl::disable(GL_BLEND);
        // gl::blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        auto &program = getBindProgram("TAA");
        program.set_sampler2D("un_curr_color", m_lightbuffers[0]->attachments[0]);
        program.set_sampler2D("un_prev_color", m_lightbuffers[2]->attachments[0]);

        program.set_sampler2D("un_curr_depth", m_gbuffers[0]->attachments[4]);
        program.set_sampler2D("un_prev_depth", m_gbuffers[1]->attachments[4]);
    
        program.set_sampler2D("un_velocity",   m_gbuffers[0]->attachments[3]);
        program.set_sampler2D("un_prev_velocity", m_gbuffers[1]->attachments[3]);

        gl::drawArrays(GL_TRIANGLES, 0, 6);
    }


    IDK_GLCALL(
        glBlitNamedFramebuffer(
            m_lightbuffers[1]->m_FBO,
            m_lightbuffers[2]->m_FBO,
            0, 0, width(), height(),
            0, 0, width(), height(),
            GL_COLOR_BUFFER_BIT,
            GL_NEAREST
        );
    )


    PostProcess_bloom(*(m_lightbuffers[1]));
    // std::swap(src, dst);

    // // PostProcess_chromatic_aberration(*A, *B);
    // // std::swap(src, dst);

    // PostProcess_colorgrading(camera, buffer_in, *m_lightbuffers[0]);
    // // std::swap(src, dst);


    buffer_out.bind();
    buffer_out.clear(GL_COLOR_BUFFER_BIT);

    {
        auto &program = getBindProgram("colorgrade");
        program.set_sampler2D("un_input", m_lightbuffers[1]->attachments[0]);
        program.set_sampler2D("un_bloom", m_bloom_buffers[0].attachments[0]);
        gl::drawArrays(GL_TRIANGLES, 0, 6);   
    }



    // -----------------------------------------------------------------------------------------


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
                0, 0, m_winsize.x, m_winsize.y,
                0, 0, m_winsize.x, m_winsize.y,
                GL_COLOR_BUFFER_BIT,
                GL_NEAREST
            );
        )
    }


    gl::enable(GL_DEPTH_TEST, GL_CULL_FACE);
}
