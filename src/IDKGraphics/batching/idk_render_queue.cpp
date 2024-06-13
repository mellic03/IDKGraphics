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


const std::vector<idk::glDrawCmd> &
idk::RenderQueue::genDrawCommands( idk::DrawIndirectData &data, idk::ModelAllocator &MA )
{
    static std::vector<idk::glDrawCmd> commands(idk::indirect_draw::MAX_DRAW_CALLS);
    commands.resize(0);

    size_t material_offset  = 0;
    size_t transform_offset = 0;
    size_t drawID_offset    = 0;

    for (auto &[model_id, model_transforms]: m_drawlist)
    {
        for (int mesh_id: MA.getModel(model_id).mesh_ids)
        {
            idk::MeshDescriptor &mesh = MA.getMesh(mesh_id);


            data.offsets[drawID_offset++] = transform_offset;

            for (glm::mat4 &T: model_transforms)
            {
                // Transform will be un_offsets[gl_DrawID] + gl_InstanceID
                data.transforms[transform_offset++] = T;
            }


            for (int texture_id: MA.getMaterial(mesh.material_id).textures)
            {
                GLuint64 handle = MA.getTexture(texture_id).handle;
                data.materials[material_offset++] = handle;   
            }

            commands.push_back(genDrawCommand(model_id, mesh));
        }
    }


    return commands;
}



const std::vector<idk::glDrawCmd> &
idk::RenderQueue::genDrawCommands( idk::DrawIndirectData &data, idk::ModelAllocator &MA, idk::Camera &cam )
{
    static std::vector<idk::glDrawCmd> commands(idk::indirect_draw::MAX_DRAW_CALLS);
    commands.resize(0);


    auto frustum = idk::geometry::createCameraFrustum(
        cam.near, cam.far,
        cam.fov, cam.aspect,
        cam.position,
        cam.getFront(),
        cam.getUp(),
        cam.getRight()
    );


    size_t material_offset  = 0;
    size_t transform_offset = 0;
    size_t drawID_offset    = 0;


    for (auto &[model_id, model_transforms]: m_drawlist)
    {
        for (int mesh_id: MA.getModel(model_id).mesh_ids)
        {
            idk::MeshDescriptor &mesh = MA.getMesh(mesh_id);
            // bool culled = true;
            size_t num_instances = 0;

            for (glm::mat4 &T: model_transforms)
            {
                if (idk::geometry::inFrustum(frustum, T, mesh.bounding_radius) == false)
                {
                    continue;
                }

                // if (culled)
                // {
                //     culled = false;
                    data.offsets[drawID_offset++] = transform_offset;
                // }

                // Transform offset will be un_offsets[gl_DrawID] + gl_InstanceID
                num_instances += 1;
                data.transforms[transform_offset++] = T;
            }


            // if (culled)
            // {
            //     continue;
            // }


            for (int texture_id: MA.getMaterial(mesh.material_id).textures)
            {
                GLuint64 handle = MA.getTexture(texture_id).handle;
                data.materials[material_offset++] = handle;   
            }

            commands.push_back(genDrawCommand(model_id, mesh, num_instances));
        }
    }


    return commands;
}
