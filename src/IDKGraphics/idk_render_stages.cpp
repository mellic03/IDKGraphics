#include "idk_renderengine.hpp"
#include "render/idk_vxgi.hpp"

#include <libidk/idk_noisegen.hpp>


void
idk::RenderEngine::RenderStage_geometry( idk::Camera &camera, float dtime,
                                         glFramebuffer &buffer_out )
{
    buffer_out.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    buffer_out.bind();


    gl::disable(GL_CULL_FACE);

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

    {
        idk::RenderQueue &queue = _getRenderQueue(m_viewspace_RQ);

        const auto &commands = queue.genDrawCommands(*m_DrawIndirectData, m_model_allocator);
        m_DrawCommandBuffer.bufferSubData(0, commands.size()*sizeof(idk::glDrawCmd), commands.data());
        m_DrawIndirectSSBO.update(*m_DrawIndirectData);

        auto &program = getProgram("gpass-viewspace");
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

    idk::glDepthCascade depthcascade = m_lightsystem.depthCascade();
    program.set_vec4("un_cascade_depths", depthcascade.getCascadeDepths(camera.far));
    program.set_sampler2DArray("un_dirlight_depthmap", depthcascade.getTextureArray());
    program.set_sampler2D("un_fragdepth", m_geom_buffer.depth_attachment);

    tex2tex(program, buffer_in, buffer_out);
}



void
idk::RenderEngine::RenderStage_pointlights()
{
    idk::glDrawCmd cmd = genLightsourceDrawCommand(m_unit_sphere, m_pointlights.size(), modelAllocator());
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
    gl::bindVertexArray(m_quad_VAO);

    buffer_out.bind();
    buffer_out.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    // Lighting pass
    // -----------------------------------------------------------------------------------------
    gl::disable(GL_DEPTH_TEST, GL_CULL_FACE);

    glShaderProgram &program = getProgram("lpass");
    program.bind();
    program.set_sampler2D("un_fragdepth", m_geom_buffer.depth_attachment);
    program.set_sampler2D("un_BRDF_LUT", BRDF_LUT);

    idk::glDepthCascade depthcascade = m_lightsystem.depthCascade();
    program.set_vec4("un_cascade_depths", depthcascade.getCascadeDepths(camera.far));
    program.set_sampler2DArray("un_dirlight_depthmap", depthcascade.getTextureArray());

    program.set_samplerCube("un_skybox_atmosphere", m_skybox);
    program.set_samplerCube("un_skybox_diffuse", skyboxes_IBL[current_skybox].first);
    program.set_samplerCube("un_skybox_specular", skyboxes_IBL[current_skybox].second);

    tex2tex(program, buffer_in, buffer_out);
    program.popTextureUnits();
    // -----------------------------------------------------------------------------------------


    gl::bindVertexArray(m_model_allocator.getVAO());

    gl::enable(GL_CULL_FACE, GL_BLEND);
    gl::cullFace(GL_FRONT);

    IDK_GLCALL( glBlendFunc(GL_ONE, GL_ONE); )
    RenderStage_spotlights();
    RenderStage_pointlights();


    gl::disable(GL_DEPTH_TEST);
    RenderStage_atmospheres(camera, buffer_in, buffer_out);
    // RenderStage_volumetrics(camera, buffer_in, m_scratchbuffers[1]);

    gl::disable(GL_CULL_FACE);
    gl::cullFace(GL_BACK);

    // // Combine geometry and volumetrics
    // // -----------------------------------------------------------------------------------------
    // gl::bindVertexArray(m_quad_VAO);

    // IDK_GLCALL( glBlendFunc(GL_SRC_ALPHA, GL_ONE); )

    // glShaderProgram &additive = getProgram("additive");
    // additive.bind();
    // tex2tex(additive, m_scratchbuffers[0], buffer_out);

    gl::disable(GL_BLEND);
    // // -----------------------------------------------------------------------------------------

}


void
idk::RenderEngine::PostProcess_chromatic_aberration( glFramebuffer &buffer_in,
                                                     glFramebuffer &buffer_out )
{
    glShaderProgram &program = getProgram("chromatic");
    program.bind();
    tex2tex(program, buffer_in, buffer_out);
}


void
idk::RenderEngine::PostProcess_SSR()
{

}


void
idk::RenderEngine::PostProcess_bloom( glFramebuffer &buffer_in,
                                      glFramebuffer &buffer_out )
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
idk::RenderEngine::RenderStage_postprocessing( idk::Camera &camera,
                                               glFramebuffer &buffer_in,
                                               glFramebuffer &buffer_out )
{
    // PostProcess_chromatic_aberration(buffer_in, m_mainbuffer_0);

    PostProcess_bloom(buffer_in, m_mainbuffer_1);
    PostProcess_colorgrading(camera, buffer_in, m_mainbuffer_0);

    // FXAA
    // -----------------------------------------------------------------------------------------
    glShaderProgram &fxaa = getProgram("fxaa");
    fxaa.bind();
    tex2tex(fxaa, m_mainbuffer_0, buffer_out);
    // -----------------------------------------------------------------------------------------

    gl::bindFramebuffer(GL_FRAMEBUFFER, 0);

}
