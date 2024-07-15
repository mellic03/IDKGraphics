#include "../idk_renderengine.hpp"
#include "cubemap.hpp"



unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;

void renderCube()
{
    // initialize (if necessary)
    if (cubeVAO == 0)
    {
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glBindVertexArray(cubeVAO);
    }
    // render Cube
    glDrawArrays(GL_TRIANGLES, 0, 36);
}


// inline static glm::vec3 PROBE_CELL_SPACING = glm::vec3(2.0, 3.0, 2.0);
// inline static uint32_t PROBE_GRID_X = 8;
// inline static uint32_t PROBE_GRID_Y = 3;
// inline static uint32_t PROBE_GRID_Z = 8;
// inline static uint32_t PROBE_GRID_NPROBES = PROBE_GRID_X * PROBE_GRID_Y * PROBE_GRID_Z;
// inline static auto PROBE_GRID_SIZE  = glm::ivec3(PROBE_GRID_X, PROBE_GRID_Y, PROBE_GRID_Z);
// inline static auto PROBE_GRID_HSIZE = PROBE_GRID_SIZE / 2;


glm::vec3
idk::env_probe::layerToWorld( const glm::vec3 &viewpos, uint32_t layer,
                              const EnvProbeSettings &config )
{
    const int w = config.grid_size.x;
    const int h = config.grid_size.y;
    const int d = config.grid_size.z;

    int x = layer % w;
    int y = (layer / w) % h;
    int z = layer / (w * h);

    glm::vec3 world = glm::vec3(x, y, z);
              world -= glm::vec3(config.grid_size / 2);
              world *= config.cell_size;

    return world;
}


uint32_t
idk::env_probe::worldToLayer( const glm::vec3 &viewpos, glm::vec3 world,
                              const EnvProbeSettings &config )
{
    const int w = config.grid_size.x;
    const int h = config.grid_size.y;
    const int d = config.grid_size.z;

    glm::ivec3 coord = glm::round(world);
    uint32_t layer = w*h*coord.z + w*coord.y + coord.x;

    return glm::clamp(layer, uint32_t(0), uint32_t(w*h*d));
}




void
idk::RenderEngine::_render_cubemap( const glm::vec3 &pos, uint32_t layer,
                                    idk::glShaderProgram &program,
                                    idk::RenderQueue &queue, idk::glFramebuffer &buffer_out )
{
    if (queue.numDrawCommands() == 0)
    {
        return;
    }

    const glm::mat4 P = glm::perspective(glm::radians(90.0f), 1.0f, 0.001f, 64.0f);

    const glm::mat4 V[6] = {
        glm::lookAt(pos, pos+glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(pos, pos+glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(pos, pos+glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(pos, pos+glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(pos, pos+glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(pos, pos+glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    program.set_uint("un_draw_offset", uint32_t(queue.getDrawCommandOffset()));

    for (int face=0; face<6; face++)
    {
        program.set_mat4("un_PV", P * V[face]);

        gl::namedFramebufferTextureLayer(
            buffer_out.m_FBO,
            GL_COLOR_ATTACHMENT0,
            buffer_out.attachments[0],
            0,
            6*layer + face
        );

        gl::namedFramebufferTextureLayer(
            buffer_out.m_FBO,
            GL_DEPTH_ATTACHMENT,
            buffer_out.depth_attachment,
            0,
            6*layer + face
        );

        buffer_out.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        gl::multiDrawElementsIndirect(
            GL_TRIANGLES,
            GL_UNSIGNED_INT,
            (const void *)(sizeof(idk::glDrawCmd) * queue.getDrawCommandOffset()),
            queue.numDrawCommands(),
            sizeof(idk::glDrawCmd)
        );
    }

    gl::generateTextureMipmap(buffer_out.attachments[0]);

}



void
idk::RenderEngine::_render_envmap( const glm::vec3 &pos, uint32_t face,
                                   idk::RenderQueue &queue, idk::glFramebuffer &buffer_out )
{
    if (queue.numDrawCommands() == 0)
    {
        return;
    }

    const glm::mat4 P = glm::perspective(glm::radians(90.0f), 1.0f, 0.001f, 32.0f);

    {
        gl::namedFramebufferTextureLayer(
            buffer_out.m_FBO,
            GL_COLOR_ATTACHMENT0,
            buffer_out.attachments[0],
            0,
            face
        );

        gl::namedFramebufferTextureLayer(
            buffer_out.m_FBO,
            GL_DEPTH_ATTACHMENT,
            buffer_out.depth_attachment,
            0,
            face
        );

        buffer_out.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }


    // {
    //     constexpr glm::vec3 zero = glm::vec3(0.0f);

    //     const glm::mat4 V[6] = {
    //         glm::lookAt(zero, glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
    //         glm::lookAt(zero, glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
    //         glm::lookAt(zero, glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
    //         glm::lookAt(zero, glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
    //         glm::lookAt(zero, glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
    //         glm::lookAt(zero, glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    //     };

    //     auto &program = getProgram("envprobe-background");
    //     program.bind();
    //     program.set_mat4("un_PV", P * V[face]);
    //     program.set_samplerCube("un_skybox", skyboxes[current_skybox]);

    //     if (cubeVAO != 0)
    //     {
    //         gl::bindVertexArray(cubeVAO); 
    //     }
    //     renderCube();

    //     gl::bindVertexArray(m_model_allocator.getVAO());
    // }



    // gl::enable(GL_BLEND);
    // gl::blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    {
        const glm::mat4 V[6] = {
            glm::lookAt(pos, pos+glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(pos, pos+glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(pos, pos+glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
            glm::lookAt(pos, pos+glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
            glm::lookAt(pos, pos+glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(pos, pos+glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
        };

        auto &program = getProgram("envprobe-cubemap");
        program.bind();

        program.set_sampler2DArray("un_shadowmap", m_dirshadow_buffer.depth_attachment);
        program.set_sampler2D("un_BRDF_LUT",  BRDF_LUT);
        program.set_uint("un_draw_offset", uint32_t(queue.getDrawCommandOffset()));
        program.set_mat4("un_projection", P);
        program.set_mat4("un_view", V[face]);
        program.set_vec3("un_viewpos", pos);


        gl::multiDrawElementsIndirect(
            GL_TRIANGLES,
            GL_UNSIGNED_INT,
            (const void *)(sizeof(idk::glDrawCmd) * queue.getDrawCommandOffset()),
            queue.numDrawCommands(),
            sizeof(idk::glDrawCmd)
        );
    }

    gl::disable(GL_BLEND);

}


void
idk::RenderEngine::_convolve_cubemap( uint32_t cubemap, uint32_t layer, uint32_t face,
                                      idk::glShaderProgram &program,
                                      idk::glFramebuffer &buffer_out )
{
    const glm::mat4 P = glm::perspective(glm::radians(90.0f), 1.0f, 0.001f, 32.0f);

    const glm::mat4 V[6] = {
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    // environment cubemap --> convolved cubemap
    // -----------------------------------------------------------------------------------------
    buffer_out.bind();

    program.set_samplerCube("un_cubemap",  cubemap);
    program.set_samplerCube("un_depthmap", m_envprobe_buffer.depth_attachment);

    // for (int face=0; face<6; face++)
    {
        program.set_mat4("un_PV", P * V[face]);

        gl::namedFramebufferTextureLayer(
            buffer_out.m_FBO,
            GL_COLOR_ATTACHMENT0,
            buffer_out.attachments[0],
            0,
            6*layer + face
        );

        buffer_out.clear(GL_COLOR_BUFFER_BIT);

        if (cubeVAO != 0)
        {
            gl::bindVertexArray(cubeVAO); 
        }
        renderCube();
    }

    // -----------------------------------------------------------------------------------------

}



