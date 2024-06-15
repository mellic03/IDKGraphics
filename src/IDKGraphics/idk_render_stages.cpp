#include "idk_renderengine.hpp"
#include "render/idk_vxgi.hpp"

#include <libidk/idk_noisegen.hpp>


void
idk::RenderEngine::RenderStage_geometry( idk::Camera &camera, float dtime,
                                         glFramebuffer &buffer_out )
{
    buffer_out.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    buffer_out.bind();

    gl::enable(GL_CULL_FACE);

    // Internal geometry pass render queue
    // -----------------------------------------------------------------------------------------
    {
        idk::RenderQueue &queue = _getRenderQueue(m_RQ);

        const auto &commands = queue.genDrawCommands(*m_DrawIndirectData, m_model_allocator);
        m_DrawCommandBuffer.bufferSubData(0, commands.size()*sizeof(idk::glDrawCmd), commands.data());
        m_DrawIndirectSSBO.update(*m_DrawIndirectData);

        auto &program = getProgram("gpass");
        program.bind();

        gl::multiDrawElementsIndirect(
            GL_TRIANGLES,
            GL_UNSIGNED_INT,
            nullptr,
            commands.size(),
            sizeof(idk::glDrawCmd)
        );
    }
    // -----------------------------------------------------------------------------------------


    // User-created render queues
    // -----------------------------------------------------------------------------------------
    for (idk::RenderQueue &queue: m_user_render_queues)
    {
        const auto &commands = queue.genDrawCommands(*m_DrawIndirectData, m_model_allocator);

        if (commands.size() == 0)
        {
            continue;
        }

        m_DrawCommandBuffer.bufferSubData(0, commands.size()*sizeof(idk::glDrawCmd), commands.data());
        m_DrawIndirectSSBO.update(*m_DrawIndirectData);

        auto &program = getProgram(queue.name);
        program.bind();

        gl::multiDrawElementsIndirect(
            GL_TRIANGLES,
            GL_UNSIGNED_INT,
            nullptr,
            commands.size(),
            sizeof(idk::glDrawCmd)
        );
    }
    // -----------------------------------------------------------------------------------------



    // Heighmap terrain
    // -----------------------------------------------------------------------------------------
    idk::RenderQueue &queue = _getRenderQueue(m_viewspace_RQ);

    // -----------------------------------------------------------------------------------------



    // Particles
    // -----------------------------------------------------------------------------------------
    // gl::enable(GL_BLEND);
    // IDK_GLCALL( glDepthMask(GL_FALSE); )
    // IDK_GLCALL( glBlendFunc(GL_SRC_ALPHA, GL_ONE); )
    {
        idk::RenderQueue &queue = _getRenderQueue(m_viewspace_RQ);

        const auto &commands = queue.genDrawCommands(*m_DrawIndirectData, m_model_allocator);
        m_DrawCommandBuffer.bufferSubData(0, commands.size()*sizeof(idk::glDrawCmd), commands.data());
        m_DrawIndirectSSBO.update(*m_DrawIndirectData);

        auto &program = getProgram("gpass-particle");
        program.bind();

        gl::multiDrawElementsIndirect(
            GL_TRIANGLES,
            GL_UNSIGNED_INT,
            nullptr,
            commands.size(),
            sizeof(idk::glDrawCmd)
        );
    }
    // glDepthMask(GL_TRUE);
    // -----------------------------------------------------------------------------------------

}



void
idk::RenderEngine::RenderStage_atmospheres( idk::Camera   &camera,
                                            glFramebuffer &buffer_in,
                                            glFramebuffer &buffer_out )
{
    idk::glDrawCmd cmd = genAtmosphereDrawCommand(modelAllocator());
    if (cmd.instanceCount == 0)
    {
        return;
    }
    m_DrawCommandBuffer.bufferSubData(0, sizeof(idk::glDrawCmd), &cmd);

    auto &program = getProgram("atmosphere");
    program.bind();

    program.set_sampler2D("un_texture_0", m_geom_buffer.attachments[0]);
    program.set_sampler2D("un_texture_1", m_geom_buffer.attachments[1]);
    program.set_sampler2D("un_texture_2", m_geom_buffer.attachments[2]);

    program.set_sampler2D("un_fragdepth", m_geom_buffer.depth_attachment);
    program.set_sampler2D("un_BRDF_LUT", BRDF_LUT);

    gl::multiDrawElementsIndirect(
        GL_TRIANGLES,
        GL_UNSIGNED_INT,
        nullptr,
        1,
        sizeof(idk::glDrawCmd)
    );

}


void
idk::RenderEngine::RenderStage_volumetrics( idk::Camera   &camera,
                                            glFramebuffer &buffer_in,
                                            glFramebuffer &buffer_out )
{
    auto &program = getProgram("dir-volumetric");
    program.bind();

    IDK_GLCALL( glBlendFunc(GL_SRC_ALPHA, GL_ONE); )

    // idk::glDepthCascade depthcascade = m_lightsystem.depthCascade();
    // program.set_vec4("un_cascade_depths", depthcascade.getCascadeDepths(camera.far));
    // program.set_sampler2DArray("un_dirlight_depthmap", depthcascade.getTextureArray());
    program.set_sampler2D("un_fragdepth", m_geom_buffer.depth_attachment);

    tex2tex(program, buffer_in, buffer_out);
}



void
idk::RenderEngine::RenderStage_dirlights()
{
    idk::glDrawCmd cmd = genLightsourceDrawCommand(m_unit_sphere, m_dirlights.size(), modelAllocator());
    if (cmd.instanceCount == 0)
    {
        return;
    }
    m_DrawCommandBuffer.bufferSubData(0, sizeof(idk::glDrawCmd), &cmd);


    auto &program = getProgram("deferred-dirlight");
    program.bind();

    program.set_sampler2D("un_texture_0", m_geom_buffer.attachments[0]);
    program.set_sampler2D("un_texture_1", m_geom_buffer.attachments[1]);
    program.set_sampler2D("un_texture_2", m_geom_buffer.attachments[2]);
    program.set_sampler2D("un_fragdepth", m_geom_buffer.depth_attachment);
    program.set_sampler2D("un_BRDF_LUT",  BRDF_LUT);

    program.set_samplerCube("un_skybox_diffuse", skyboxes_IBL[current_skybox].first);
    program.set_samplerCube("un_skybox_specular", skyboxes_IBL[current_skybox].second);

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
idk::RenderEngine::RenderStage_pointlights()
{
    idk::glDrawCmd cmd = genLightsourceDrawCommand(
        m_unit_sphere, m_pointlights.size(), modelAllocator()
    );

    if (cmd.instanceCount == 0)
    {
        return;
    }
    m_DrawCommandBuffer.bufferSubData(0, sizeof(idk::glDrawCmd), &cmd);


    auto &program = getProgram("deferred-pointlight");
    program.bind();

    program.set_sampler2D("un_texture_0", m_geom_buffer.attachments[0]);
    program.set_sampler2D("un_texture_1", m_geom_buffer.attachments[1]);
    program.set_sampler2D("un_texture_2", m_geom_buffer.attachments[2]);
    program.set_sampler2D("un_fragdepth", m_geom_buffer.depth_attachment);
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
    idk::glDrawCmd cmd = genLightsourceDrawCommand(m_unit_sphere, m_spotlights.size(), modelAllocator());
    if (cmd.instanceCount == 0)
    {
        return;
    }
    m_DrawCommandBuffer.bufferSubData(0, sizeof(idk::glDrawCmd), &cmd);

    auto &program = getProgram("deferred-spotlight");
    program.bind();

    program.set_sampler2D("un_texture_0", m_geom_buffer.attachments[0]);
    program.set_sampler2D("un_texture_1", m_geom_buffer.attachments[1]);
    program.set_sampler2D("un_texture_2", m_geom_buffer.attachments[2]);
    program.set_sampler2D("un_fragdepth", m_geom_buffer.depth_attachment);
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
idk::RenderEngine::RenderStage_lighting( idk::Camera &camera, float dtime,
                                         glFramebuffer &buffer_in,
                                         glFramebuffer &buffer_out )
{
    m_scratchbuffers2[2].bind();
    m_scratchbuffers2[2].clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // buffer_out.bind();

    gl::bindVertexArray(m_model_allocator.getVAO());

    gl::enable(GL_CULL_FACE);
    gl::cullFace(GL_FRONT);

    gl::enable(GL_BLEND);
    IDK_GLCALL( glBlendFunc(GL_SRC_ALPHA, GL_ONE); )

    RenderStage_dirlights();
    RenderStage_pointlights();
    RenderStage_spotlights();


    buffer_out.bind();

    gl::bindVertexArray(m_quad_VAO);
    gl::disable(GL_CULL_FACE, GL_DEPTH_TEST);

    // // Do SSR before background
    // // -----------------------------------------------------------------------------------------
    // // IDK_GLCALL( glBlendFunc(GL_SRC_ALPHA, GL_ONE); )
    // gl::disable(GL_BLEND);
    // PostProcess_SSR(m_mip_scratchbuffer, m_scratchbuffers2[2]);
    // gl::enable(GL_BLEND);
    // // -----------------------------------------------------------------------------------------


    // step alpha 0/1
    // -----------------------------------------------------------------------------------------
    {
        glShaderProgram &program = getProgram("alpha-0-1");
        program.bind();
        program.set_sampler2D("un_input", m_scratchbuffers2[2].attachments[0]);

        gl::drawArrays(GL_TRIANGLES, 0, 6);
    }
    // -----------------------------------------------------------------------------------------


    // Background
    // -----------------------------------------------------------------------------------------
    IDK_GLCALL( glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_DST_ALPHA); )
    {
        glShaderProgram &program = getProgram("background");
        program.bind();
        program.set_samplerCube("un_skybox", skyboxes[current_skybox]);

        gl::drawArrays(GL_TRIANGLES, 0, 6);
    }
    // -----------------------------------------------------------------------------------------




    gl::disable(GL_DEPTH_TEST);
    // RenderStage_atmospheres(camera, buffer_in, buffer_out);
    // RenderStage_volumetrics(camera, buffer_in, m_scratchbuffers[1]);

    gl::disable(GL_CULL_FACE);
    gl::cullFace(GL_BACK);

    // Combine geometry and volumetrics
    // -----------------------------------------------------------------------------------------
    // gl::bindVertexArray(m_quad_VAO);

    // IDK_GLCALL( glBlendFunc(GL_SRC_ALPHA, GL_ONE); )

    // glShaderProgram &additive = getProgram("additive");
    // additive.bind();
    // tex2tex(additive, m_scratchbuffers[0], buffer_out);

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

    gl::generateTextureMipmap(m_mip_scratchbuffer.attachments[0]);

    buffer_out.bind();

    auto &program = getProgram("SSR");
    program.bind();

    program.set_sampler2D("un_input",  m_mip_scratchbuffer.attachments[0]);
    program.set_sampler2D("un_albedo", m_geom_buffer.attachments[0]);
    program.set_sampler2D("un_normal", m_geom_buffer.attachments[1]);
    program.set_sampler2D("un_pbr",    m_geom_buffer.attachments[2]);
    program.set_sampler2D("un_fragdepth", m_geom_buffer.depth_attachment);
    program.set_sampler2D("un_BRDF_LUT",  BRDF_LUT);
    program.set_samplerCube("un_skybox", skyboxes[current_skybox]);

    gl::drawArrays(GL_TRIANGLES, 0, 6);
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
    IDK_GLCALL( glBlendFunc(GL_ONE, GL_ONE); )

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
idk::RenderEngine::PostProcess_colorgrading( idk::Camera &camera,
                                             glFramebuffer &buffer_in,
                                             glFramebuffer &buffer_out )
{
    glShaderProgram &program = getProgram("colorgrade");
    program.bind();
    program.set_sampler2D("un_bloom", m_bloom_buffers[0].attachments[0]);

    tex2tex(program, buffer_in, buffer_out);
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
idk::RenderEngine::RenderStage_postprocessing( idk::Camera   &camera,
                                               glFramebuffer &buffer_in,
                                               glFramebuffer &buffer_out )
{
    idk::glFramebuffer *src = &m_scratchbuffers2[0];
    idk::glFramebuffer *dst = &m_scratchbuffers2[1];

    gl::disable(GL_DEPTH_TEST);

    PostProcess_SSR(buffer_in, *dst);
    PostProcess_bloom(*dst);
    std::swap(src, dst);

    // PostProcess_chromatic_aberration(*A, *B);
    // std::swap(src, dst);

    PostProcess_colorgrading(camera, *src, *dst);
    std::swap(src, dst);

    // FXAA
    // -----------------------------------------------------------------------------------------
    glShaderProgram &fxaa = getProgram("fxaa");
    fxaa.bind();
    tex2tex(fxaa, *src, buffer_out);
    // -----------------------------------------------------------------------------------------

    PostProcess_text(buffer_out);
    PostProcess_overlay(buffer_out);



    glShaderProgram &program = getProgram("screenquad");
    program.bind();
    f_fbfb(program, buffer_out);

}
