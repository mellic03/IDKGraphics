#include "../idk_renderengine.hpp"



void
idk::RenderEngine::shadowpass_dirlights()
{
    idk::Camera &cam = getCamera();

    std::vector<idk::Dirlight> &dirlights = m_lightsystem.dirlights();
    idk::glDepthCascade &depthcascade = m_lightsystem.depthCascade();

    idk::Dirlight &light = dirlights[0];
    const glm::vec3 dir = glm::normalize(glm::vec3(light.direction));


    // Compute matrices
    // -------------------------------------------------------------------------------------
    depthcascade.bind();

    depthcascade.computeCascadeMatrices(
        cam.fov,    cam.aspect,  cam.near,
        cam.far,  cam.V(),       dir
    );

    const auto &cascade_matrices = depthcascade.getCascadeMatrices();
    // -------------------------------------------------------------------------------------

    idk::RenderQueue &queue = _getRenderQueue(m_shadow_RQ);
    const auto &commands = queue.genDrawCommands(*m_DrawIndirectData, m_model_allocator, getCamera());

    m_DrawCommandBuffer.bufferSubData(0, commands.size()*sizeof(idk::glDrawCmd), commands.data());
    m_DrawIndirectSSBO.update(*m_DrawIndirectData);


    auto &program = getProgram("dirshadow-indirect");
    program.bind();

    for (int i=0; i<glDepthCascade::NUM_CASCADES; i++)
    {
        depthcascade.setOutputAttachment(i);
        depthcascade.clear(GL_DEPTH_BUFFER_BIT);

        program.set_mat4("un_lightspacematrix", cascade_matrices[i]);

        gl::multiDrawElementsIndirect(
            GL_TRIANGLES,
            GL_UNSIGNED_INT,
            nullptr,
            commands.size(),
            sizeof(idk::glDrawCmd)
        );
    }


    program.unbind();
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

