#include "idk_render_queue.hpp"

#include <libidk/idk_geometry.hpp>
#include <libidk/idk_transform.hpp>



void
idk::RenderQueue::enque( int model, const glm::mat4 &transform )
{
    m_drawlist[model].push_back({transform, transform});
}


void
idk::RenderQueue::enque( int model, const glm::mat4 &transform, const glm::mat4 &prev )
{
    m_drawlist[model].push_back({transform, prev});
}


void
idk::RenderQueue::enque( int model, const idk::Transform &T, const IDK_Camera &camera,
                         ModelAllocator &MA )
{
    // How many multiples of radius is the mesh from the camera?
    float rad  = MA.getModel(model).bounding_radius; // * mesh.bounding_radius;
    float dist = glm::distance(glm::vec3(camera.position), T.position);

    float radii = dist / rad;
    int   level = radii / draw_buffer::MODEL_MAX_LOD;
          level = glm::clamp(level, 0, draw_buffer::MODEL_MAX_LOD);

    idk_printvalue(rad);
    idk_printvalue(dist);
    idk_printvalue(radii);
    idk_printvalue(level);
    std::cout << "\n";

    int lod_id = MA.getModel(model).LOD[level];

    m_drawlist[lod_id].push_back({Transform::toGLM(T), Transform::toGLM(T)});
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
        uint32_t og_texture_offset = texture_offset;

        for (auto &mesh: MA.getModel(model_id).meshes)
        {
            for (GLuint64 handle: MA.getMaterial(mesh.material).handles)
            {
                ssbo_buffer.textures[texture_offset] = handle;
                texture_offset += 1;
            }
        }


        uint32_t og_transform_offset = transform_offset;
    
        for (auto &[T, prev]: model_transforms)
        {
            ssbo_buffer.transforms[transform_offset] = T;
            ssbo_buffer.prev_transforms[transform_offset] = prev;
            transform_offset += 1;
        }


        for (auto &[T, prev]: model_transforms)
        {
            uint32_t trans_off = 0;
            uint32_t tex_off = 0;

            for (auto &mesh: MA.getModel(model_id).meshes)
            {
                ssbo_buffer.transform_offsets[drawID_offset] = og_transform_offset + trans_off;
                ssbo_buffer.texture_offsets[drawID_offset]   = og_texture_offset   + tex_off;
                drawID_offset += 1;
                tex_off += MA.getMaterial(mesh.material).handles.size();

                commands.push_back(genDrawCommand(model_id, mesh));
            }

            trans_off += 1;
        }
    }

    m_num_drawcmd = commands.size() - m_drawcmd_offset;
}


// void
// idk::RenderQueue::genDrawCommands( idk::ModelAllocator         &MA,
//                                    size_t                      &texture_offset,
//                                    size_t                      &transform_offset,
//                                    size_t                      &drawID_offset,
//                                    idk::SSBO_Buffer            &ssbo_buffer,
//                                    std::vector<idk::glDrawCmd> &commands )
// {
//     m_drawcmd_offset = commands.size();

//     for (auto &[model_id, model_transforms]: m_drawlist)
//     {
//         for (MeshDescriptor &mesh: MA.getModel(model_id).meshes)
//         {
//             ssbo_buffer.transform_offsets[drawID_offset] = transform_offset;
//             ssbo_buffer.texture_offsets[drawID_offset]  = texture_offset;
//             drawID_offset += 1;

//             for (glm::mat4 &T: model_transforms)
//             {
//                 // Transform will be un_offsets[gl_DrawID] + gl_InstanceID
//                 ssbo_buffer.transforms[transform_offset] = T;
//                 transform_offset += 1;
//             }

//             for (GLuint64 handle: mesh.handles)
//             {
//                 ssbo_buffer.textures[texture_offset] = handle;
//                 texture_offset += 1;
//             }

//             commands.push_back(genDrawCommand(model_id, mesh));
//         }
//     }

//     m_num_drawcmd = commands.size() - m_drawcmd_offset;
// }

