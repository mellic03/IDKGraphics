#include "../idk_renderengine.hpp"



void
idk::RenderEngine::shadowpass_dirlights()
{
    idk::Camera &cam = getCamera();

    m_dirshadow_buffer.clear(GL_DEPTH_BUFFER_BIT);
    m_dirshadow_buffer.bind();

    for (auto &light: m_dirlights)
    {
        idk::RenderQueue &queue = _getRenderQueue(m_shadow_RQ);
        
        const auto &commands = queue.genDrawCommands(
            *m_DrawIndirectData, m_model_allocator
        );

        if (commands.size() == 0)
        {
            continue;
        }

        m_DrawCommandBuffer.bufferSubData(
            0, commands.size()*sizeof(idk::glDrawCmd), commands.data()
        );

        m_DrawIndirectSSBO.update(*m_DrawIndirectData);


        auto &program = getProgram("dirshadow-indirect");
        program.bind();

        program.set_mat4("un_lightspacematrix", light.transform);

        gl::multiDrawElementsIndirect(
            GL_TRIANGLES,
            GL_UNSIGNED_INT,
            nullptr,
            commands.size(),
            sizeof(idk::glDrawCmd)
        );
    }



    // User-created shadow render queues
    // -----------------------------------------------------------------------------------------
    auto &light = getDirlight(0);

    for (idk::RenderQueue &queue: m_user_shadow_queues)
    {
        const auto &commands = queue.genDrawCommands(
            *m_DrawIndirectData, m_model_allocator
        );

        if (commands.size() == 0)
        {
            continue;
        }

        m_DrawCommandBuffer.bufferSubData(
            0, commands.size()*sizeof(idk::glDrawCmd), commands.data()
        );

        m_DrawIndirectSSBO.update(*m_DrawIndirectData);


        auto &program = getProgram(queue.name);
        program.bind();
        program.set_mat4("un_lightspacematrix", light.transform);

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
idk::RenderEngine::shadowpass_pointlights()
{

}



void
idk::RenderEngine::shadowpass_spotlights()
{

}



void
idk::RenderEngine::shadowpass()
{
    shadowpass_dirlights();
    shadowpass_pointlights();
    shadowpass_spotlights();
}

