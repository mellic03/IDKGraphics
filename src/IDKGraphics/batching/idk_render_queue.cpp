#include "idk_render_queue.hpp"

#include <libidk/idk_geometry.hpp>



void
idk::RenderQueue::enque( int model, const glm::mat4 &transform )
{
    m_drawlist[model].push_back(transform);
}



void
idk::RenderQueue::clear()
{
    m_drawlist.clear();
}



idk::glDrawCmd
idk::RenderQueue::genDrawCommand( int model_id, idk::MeshDescriptor &desc )
{
    idk::glDrawCmd cmd = {
        .count           = desc.numIndices,
        .instanceCount   = uint32_t(m_drawlist[model_id].size()),
        .firstIndex      = desc.firstIndex,
        .baseVertex      = desc.baseVertex,
        .baseInstance    = 0
    };

    return cmd;
}


idk::glDrawCmd
idk::RenderQueue::genDrawCommand( int model_id, idk::MeshDescriptor &desc, size_t n )
{
    idk::glDrawCmd cmd = {
        .count           = desc.numIndices,
        .instanceCount   = uint32_t(n),
        .firstIndex      = desc.firstIndex,
        .baseVertex      = desc.baseVertex,
        .baseInstance    = 0
    };

    return cmd;
}



void
idk::RenderQueue::genDrawCommands( idk::ModelAllocator         &MA,
                                   size_t                      &texture_offset,
                                   size_t                      &transform_offset,
                                   size_t                      &drawID_offset,
                                   idk::SSBO_Buffer            &ssbo_buffer,
                                   std::vector<idk::glDrawCmd> &commands )
{
    m_drawcmd_offset = commands.size();

    for (auto &[model_id, model_transforms]: m_drawlist)
    {
        for (MeshDescriptor &mesh: MA.getModel(model_id).meshes)
        {
            ssbo_buffer.transform_offsets[drawID_offset] = transform_offset;
            ssbo_buffer.texture_offsets[drawID_offset]  = texture_offset;
            drawID_offset += 1;

            for (glm::mat4 &T: model_transforms)
            {
                // Transform will be un_offsets[gl_DrawID] + gl_InstanceID
                ssbo_buffer.transforms[transform_offset] = T;
                transform_offset += 1;
            }

            for (GLuint64 handle: mesh.handles)
            {
                ssbo_buffer.textures[texture_offset] = handle;
                texture_offset += 1;
            }

            commands.push_back(genDrawCommand(model_id, mesh));
        }
    }

    m_num_drawcmd = commands.size() - m_drawcmd_offset;
}

