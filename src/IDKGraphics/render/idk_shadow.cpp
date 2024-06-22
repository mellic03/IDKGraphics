#include "../idk_renderengine.hpp"



void
idk::RenderEngine::shadowpass_dirlights()
{
    for (int i=0; i<1; i++)
    {
        idk::RenderQueue &queue = _getRenderQueue(m_shadow_RQ);

        if (queue.numDrawCommands() == 0)
        {
            continue;
        }

        m_dirshadow_buffer.clear(GL_DEPTH_BUFFER_BIT);
        m_dirshadow_buffer.bind();

        auto &program = getProgram("dirshadow-indirect");
        program.bind();
        program.set_uint("un_draw_offset", uint32_t(queue.getDrawCommandOffset()));


        uint32_t light_id = 0;

        for (auto &light: m_dirlights)
        {
            program.set_uint("un_light_id", light_id);

            gl::multiDrawElementsIndirect(
                GL_TRIANGLES,
                GL_UNSIGNED_INT,
                (const void *)(sizeof(idk::glDrawCmd) * queue.getDrawCommandOffset()),
                queue.numDrawCommands(),
                sizeof(idk::glDrawCmd)
            );

            light_id += 1;
        }
    }


    // User-created shadow render queues
    // -----------------------------------------------------------------------------------------
    for (idk::RenderQueue &queue: m_user_shadow_queues)
    {
        if (queue.numDrawCommands() == 0)
        {
            continue;
        }

        auto &program = getProgram(queue.name);
        program.bind();
        program.set_uint("un_draw_offset", uint32_t(queue.getDrawCommandOffset()));

        uint32_t light_id = 0;

        for (auto &light: m_dirlights)
        {
            program.set_uint("un_light_id", light_id);

            gl::multiDrawElementsIndirect(
                GL_TRIANGLES,
                GL_UNSIGNED_INT,
                (const void *)(sizeof(idk::glDrawCmd) * queue.getDrawCommandOffset()),
                queue.numDrawCommands(),
                sizeof(idk::glDrawCmd)
            );

            light_id += 1;
        }
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

