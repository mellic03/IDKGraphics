#include "../idk_renderengine.hpp"



void
idk::RenderEngine::shadowpass_dirlights()
{
    idk::RenderQueue &queue = _getRenderQueue(m_shadow_RQ);

    if (queue.numDrawCommands() == 0)
    {
        return;
    }

    if (m_dirlights.size() == 0)
    {
        return;
    }

    auto &light = m_dirlights.get(0);

    m_dirshadow_buffer.bind();

    auto &program = getProgram("dirshadow-indirect");
    program.bind();
    program.set_uint("un_draw_offset", uint32_t(queue.getDrawCommandOffset()));
    program.set_uint("un_light_id", 0);


    for (uint32_t layer=0; layer<5; layer++)
    {
        program.set_uint("un_cascade", layer);

        gl::namedFramebufferTextureLayer(
            m_dirshadow_buffer.m_FBO,
            GL_DEPTH_ATTACHMENT,
            m_dirshadow_buffer.depth_attachment,
            0,
            layer
        );

        m_dirshadow_buffer.clear(GL_DEPTH_BUFFER_BIT);

        gl::multiDrawElementsIndirect(
            GL_TRIANGLES,
            GL_UNSIGNED_INT,
            (const void *)(sizeof(idk::glDrawCmd) * queue.getDrawCommandOffset()),
            queue.numDrawCommands(),
            sizeof(idk::glDrawCmd)
        );
    }

}



// void
// idk::RenderEngine::shadowpass_dirlights()
// {
//     for (int i=0; i<1; i++)
//     {
//         idk::RenderQueue &queue = _getRenderQueue(m_shadow_RQ);

//         if (queue.numDrawCommands() == 0)
//         {
//             continue;
//         }

//         m_dirshadow_buffer.clear(GL_DEPTH_BUFFER_BIT);
//         m_dirshadow_buffer.bind();

//         auto &program = getProgram("dirshadow-indirect");
//         program.bind();
//         program.set_uint("un_draw_offset", uint32_t(queue.getDrawCommandOffset()));


//         uint32_t light_id = 0;

//         for (auto &light: m_dirlights)
//         {
//             program.set_uint("un_light_id", light_id);
//             program.set_uint("un_cascade", 0);

//             gl::namedFramebufferTextureLayer(
//                 m_dirshadow_buffer.m_FBO,
//                 GL_DEPTH_ATTACHMENT,
//                 m_dirshadow_buffer.depth_attachment,
//                 0,
//                 0
//             );

//             m_dirshadow_buffer.clear(GL_DEPTH_BUFFER_BIT);


//             gl::multiDrawElementsIndirect(
//                 GL_TRIANGLES,
//                 GL_UNSIGNED_INT,
//                 (const void *)(sizeof(idk::glDrawCmd) * queue.getDrawCommandOffset()),
//                 queue.numDrawCommands(),
//                 sizeof(idk::glDrawCmd)
//             );

//             light_id += 1;
//         }
//     }


//     // User-created shadow render queues
//     // -----------------------------------------------------------------------------------------
//     for (idk::RenderQueue &queue: m_user_shadow_queues)
//     {
//         if (queue.numDrawCommands() == 0)
//         {
//             continue;
//         }

//         auto &program = getProgram(queue.name);
//         program.bind();
//         program.set_uint("un_draw_offset", uint32_t(queue.getDrawCommandOffset()));

//         uint32_t light_id = 0;

//         for (auto &light: m_dirlights)
//         {
//             program.set_uint("un_light_id", light_id);

//             gl::multiDrawElementsIndirect(
//                 GL_TRIANGLES,
//                 GL_UNSIGNED_INT,
//                 (const void *)(sizeof(idk::glDrawCmd) * queue.getDrawCommandOffset()),
//                 queue.numDrawCommands(),
//                 sizeof(idk::glDrawCmd)
//             );

//             light_id += 1;
//         }
//     }
//     // -----------------------------------------------------------------------------------------
// }



void
idk::RenderEngine::shadowpass_pointlights()
{

}



void
idk::RenderEngine::shadowpass_spotlights()
{

}

