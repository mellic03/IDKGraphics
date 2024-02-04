#include "idk_renderengine.hpp"
#include "render/idk_drawmethods.hpp"
#include "render/idk_vxgi.hpp"

void
idk::RenderEngine::RenderStage_geometry( idk::Camera &camera, float dtime,
                                                  glFramebuffer &buffer_out )
{
    buffer_out.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    buffer_out.bind();

    // Internal render queues
    // -----------------------------------------------------------------------------------------
    for (idk::RenderQueue &rq: m_private_RQs)
    {
        glShaderProgram &rq_program = getProgram(rq.name());
        rq_program.bind();

        for (auto &[model_id, animator_id, transform]: rq)
        {
            rq.drawMethod(
                rq_program,
                model_id,
                transform,
                modelSystem()
            );
        }
    }
    // -----------------------------------------------------------------------------------------


    // User-created render queues
    // -----------------------------------------------------------------------------------------
    for (idk::RenderQueue &rq: m_public_RQs)
    {
        const auto &config = rq.config();

        bool nocull = config.cull_face == false;
        if (nocull) gl::disable(GL_CULL_FACE);

        glShaderProgram &rq_program = getProgram(rq.name());
        rq_program.bind();

        for (auto &[model_id, animator_id, transform]: rq)
        {
            rq.drawMethod(
                rq_program,
                model_id,
                transform,
                modelSystem()
            );
        }

        if (nocull) gl::enable(GL_CULL_FACE);
    }
    // -----------------------------------------------------------------------------------------

}



void
idk::RenderEngine::RenderStage_volumetrics( idk::Camera &camera,
                                            glFramebuffer &buffer_in,
                                            glFramebuffer &buffer_out )
{
    if (m_vxgi_debug)
    {
        glm::mat4 modelmat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -0.9f * camera.farPlane()));
        modelmat = glm::scale(modelmat, glm::vec3(500.0f, 500.0f, 1.0f));

        buffer_out.bind();
        auto &program = getProgram("vxgi-trace");

        program.bind();
        program.set_sampler3D("un_voxeldata", vxgi_radiance);
        program.set_mat4("un_model", modelmat);

        tex2tex(program, buffer_in, buffer_out);
    }

    else
    {
        gl::bindVertexArray(m_quad_VAO);

        buffer_out.bind();
        buffer_out.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto &program = getProgram("dir-volumetric");
        program.bind();

        idk::glDepthCascade depthcascade = m_lightsystem.depthCascade();
        program.set_vec4("un_cascade_depths", depthcascade.getCascadeDepths(camera.farPlane()));
        program.set_sampler2DArray("un_dirlight_depthmap", depthcascade.getTextureArray());

        tex2tex(program, buffer_in, buffer_out);
    }

}



void
idk::RenderEngine::RenderStage_lighting( idk::Camera &camera, float dtime,
                                                  glFramebuffer &buffer_in,
                                                  glFramebuffer &buffer_out )
{
    gl::bindVertexArray(m_quad_VAO);

    buffer_out.bind();
    buffer_out.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    gl::enable(GL_BLEND);
    IDK_GLCALL( glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); )

    // Background
    // -----------------------------------------------------------------------------------------
    glm::mat4 modelmat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -0.9f * camera.farPlane()));
    modelmat = glm::scale(modelmat, glm::vec3(500.0f, 500.0f, 1.0f));

    glShaderProgram &background = getProgram("background");
    background.bind();
    background.set_mat4("un_model", modelmat);
    background.set_samplerCube("un_skybox", skyboxes[current_skybox]);

    gl::drawArrays(GL_TRIANGLES, 0, 6);
    glShaderProgram::unbind();
    // -----------------------------------------------------------------------------------------


    // Lighting pass
    // -----------------------------------------------------------------------------------------
    gl::disable(GL_DEPTH_TEST, GL_CULL_FACE);

    glShaderProgram &program = getProgram("lpass");
    program.bind();
    program.set_sampler2D("un_BRDF_LUT", BRDF_LUT);


    // static const float     B = VXGI_WORLD_HALF_BOUNDS;
    // static const glm::mat4 P = glm::ortho(-B, B, -B, B, -B, B);

    // glm::mat4 light_matrix = P * glm::lookAt(
    //     -glm::vec3(lightSystem().getDirlight(0).direction),
    //     glm::vec3(0.0f),
    //     glm::vec3(0.0f, 1.0f, 0.0f)
    // );

    program.set_sampler3D("un_voxel_radiance",  vxgi_radiance);
    program.set_sampler3D("un_voxel_albedo",    vxgi_albedo);
    // program.set_sampler2D("un_vxgi_depthmap",  m_vxgi_buffer.depth_attachment);
    // program.set_mat4("un_vxgi_light_matrix",   light_matrix);


    // gl::bindImageTexture(0, buffer_out.attachments[0], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
    // gl::bindImageTexture(1, buffer_in.attachments[0],  0, GL_FALSE, 0, GL_READ_ONLY,  GL_RGBA16F);
    // gl::bindImageTexture(2, buffer_in.attachments[1],  0, GL_FALSE, 0, GL_READ_ONLY,  GL_RGBA32F);
    // gl::bindImageTexture(3, buffer_in.attachments[2],  0, GL_FALSE, 0, GL_READ_ONLY,  GL_RGBA16F);
    // gl::bindImageTexture(4, buffer_in.attachments[3],  0, GL_FALSE, 0, GL_READ_ONLY,  GL_RGBA16F);


    idk::glDepthCascade depthcascade = m_lightsystem.depthCascade();
    program.set_vec4("un_cascade_depths", depthcascade.getCascadeDepths(camera.farPlane()));
    program.set_sampler2DArray("un_dirlight_depthmap", depthcascade.getTextureArray());

    program.set_samplerCube("un_skybox_diffuse",  skyboxes_IBL[current_skybox].first);
    program.set_samplerCube("un_skybox_specular", skyboxes_IBL[current_skybox].second);

    // gl::dispatchCompute(this->width()/8, this->height()/8, 1);
    // IDK_GLCALL( glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT); )

    tex2tex(program, buffer_in, m_mainbuffer_0);
    program.popTextureUnits();
    // -----------------------------------------------------------------------------------------


    if (m_vxgi_debug)
    {
        RenderStage_volumetrics(camera, buffer_in, buffer_out);
    }

    else
    {
        RenderStage_volumetrics(camera, buffer_in, m_scratchbufs0[2]);

        // Combine geometry and volumetrics
        // -----------------------------------------------------------------------------------------
        glShaderProgram &additive = getProgram("additive");
        additive.bind();
        additive.set_float("intensity", 1.0f);
        tex2tex(additive, m_mainbuffer_0, m_scratchbufs0[2], buffer_out);
        // -----------------------------------------------------------------------------------------
    }

    glShaderProgram::unbind();
    gl::disable(GL_BLEND);
}


void
idk::RenderEngine::PostProcess_bloom( glFramebuffer &buffer_in, glFramebuffer &buffer_out )
{
    glShaderProgram &bloom    = getProgram("bloom");
    glShaderProgram &additive = getProgram("additive");


    bloom.bind();
    tex2tex(bloom, buffer_in, m_scratchbufs2[0]);

    // glShader &downsample = getProgram("downsample");
    // glShader &upsample   = getProgram("upsample");

    // constexpr int miplevel = 7;

    // downsample.bind();
    // tex2tex(downsample, m_scratchbufs2[0], m_scratchbufs1[1]);
    // for (int i=1; i<miplevel; i++)
    // {
    //     tex2tex(downsample, m_scratchbufs1[i], m_scratchbufs1[i+1]);
    // }

    // upsample.bind();
    // tex2tex(upsample, m_scratchbufs1[miplevel], m_scratchbufs1[miplevel], m_scratchbufs2[miplevel-1]);
    // for (int i=miplevel-1; i>0; i--)
    // {
    //     tex2tex(upsample, m_scratchbufs1[i], m_scratchbufs2[i], m_scratchbufs2[i-1]);
    // }

    additive.bind();
    additive.set_float("intensity", getCamera().m_bloom_gamma.x);
    tex2tex(additive, m_scratchbufs1[0], m_scratchbufs2[0], buffer_out);

    glShaderProgram::unbind();
}


void
idk::RenderEngine::PostProcess_chromatic_aberration( glFramebuffer &buffer_in,
                                                     glFramebuffer &buffer_out )
{
    glShaderProgram &chromatic = getProgram("chromatic");
    chromatic.bind();
    tex2tex(chromatic, buffer_in, buffer_out);
    glShaderProgram::unbind();
}


void
idk::RenderEngine::PostProcess_colorgrading( idk::Camera &camera,
                                             glFramebuffer &buffer_in,
                                             glFramebuffer &buffer_out )
{
    glShaderProgram &colorgrade = getProgram("colorgrade");
    colorgrade.bind();

    // const GLint secondmip = 4;
    // const GLint level  = log2(GL_MAX_TEXTURE_SIZE);
    // const GLint level2 = level - secondmip;

    // const size_t size   = 4;
    // const size_t size2  = size * pow(4, secondmip);
    // static float *data  = new float[size];
    // static float *data2 = new float[size2];

    // IDK_GLCALL( glGetTextureImage(buffer_in.attachments[0], level,  GL_RGBA, GL_FLOAT, size*sizeof(float),  data); )
    // IDK_GLCALL( glGetTextureImage(buffer_in.attachments[0], level2, GL_RGBA, GL_FLOAT, size2*sizeof(float), data2); )
 
    // float   texw     = m_resolution.x;
    // float   texh     = m_resolution.y;

    // float   mipw     = glm::max(1.0f, texw / float(pow(2.0f, level2)));
    // float   miph     = glm::max(1.0f, texh / float(pow(2.0f, level2)));

    // size_t  center_x = size_t(mipw / 2.0f);
    // size_t  center_y = size_t(miph / 2.0f);

    // size_t idx0 = 4 * (mipw*(center_y-1) + center_x-1);
    // size_t idx1 = 4 * (mipw*(center_y-1) + center_x  );
    // size_t idx2 = 4 * (mipw*(center_y  ) + center_x-1);
    // size_t idx3 = 4 * (mipw*(center_y  ) + center_x  );

    // // printf("%.0f  %.0f  %.0f  %.0f  %u  %u\n", texw, texh, mipw, miph, center_x, center_y);
    
    // glm::vec3   avg_color  = glm::vec3(data2[idx0+0], data2[idx0+1], data2[idx0+2]);
    //             avg_color += glm::vec3(data2[idx1+0], data2[idx1+1], data2[idx1+2]);
    //             avg_color += glm::vec3(data2[idx2+0], data2[idx2+1], data2[idx2+2]);
    //             avg_color += glm::vec3(data2[idx3+0], data2[idx3+1], data2[idx3+2]);
    //             avg_color /= 4.0f;

    // float alpha = 0.15f;
    // avg_color = alpha*avg_color + (1.0f - alpha)*glm::vec3(data[0], data[1], data[2]);

    // auto aces = [](glm::vec3 x)
    // {
    //     const float a = 2.51;
    //     const float b = 0.03;
    //     const float c = 2.43;
    //     const float d = 0.59;
    //     const float e = 0.14;
    //     glm::vec3 color = glm::clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0f, 1.0f);
    //     return color;
    // };

    // avg_color = aces(avg_color);
    // float avg_luminance = glm::dot(avg_color, glm::vec3(0.2126, 0.7152, 0.0722));

    // auto exposure_curve = [](float luminance)
    // {
    //     float a = -5.0f;
    //     float b = -0.1f;
    //     float c = -0.4f;
    //     float h =  0.9f;
    //     float v =  0.8f;
    //     float x = luminance;
    
    //     float l5 = pow(x-h, 5.0f);
    //     float l2 = pow(x-h, 2.0f);
    //     float l1 = x-h;

    //     return (a*l5 + b*l2 + c*l1 + v);
    // };

    // float new_exposure  = exposure_curve(avg_luminance);

    // float stepsize   = 1.0f / camera.m_bloom_gamma.z;
    // float difference = new_exposure - camera.m_exposure.x;
    // float direction  = difference / fabs(difference);

    // if (fabs(difference) > stepsize)
    // {
    //     camera.m_exposure.x += stepsize*direction;
    // }

    camera.m_exposure.x = 1.0f;
    tex2tex(colorgrade, buffer_in, buffer_out);
    glShaderProgram::unbind();
}



void
idk::RenderEngine::RenderStage_postprocessing( idk::Camera &camera,
                                               glFramebuffer &buffer_in,
                                               glFramebuffer &buffer_out )
{
    gl::bindVertexArray(m_quad_VAO);

    PostProcess_chromatic_aberration(buffer_in, m_mainbuffer_0);
    // PostProcess_bloom();

    auto &hist = getProgram("lum-hist");
    hist.bind();
    hist.dispatch(width()/16, height()/16, 1);


    // m_mainbuffer_0.generateMipmap(0);
    PostProcess_colorgrading(camera, m_mainbuffer_0, buffer_out);


    // FXAA
    // -----------------------------------------------------------------------------------------
    glShaderProgram &fxaa = getProgram("fxaa");
    fxaa.bind();
    f_fbfb(fxaa, buffer_out);
    // -----------------------------------------------------------------------------------------

    gl::bindVertexArray(0);
}
