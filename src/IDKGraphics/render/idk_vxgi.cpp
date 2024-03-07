#include "idk_vxgi.hpp"
 
#include <libidk/idk_texturegen.hpp>
#include <libidk/GL/idk_gltools.hpp>


GLuint
idk::VXGI::allocateTexture( size_t w )
{
    static constexpr idk::glTextureConfig config = {
        .target         = GL_TEXTURE_3D,
        .internalformat = GL_RGBA16F,
        .format         = GL_RGBA,
        .minfilter      = GL_LINEAR_MIPMAP_LINEAR,
        .magfilter      = GL_LINEAR,
        .datatype       = GL_FLOAT,
        .genmipmap      = GL_TRUE,
    };

    GLuint texture = gltools::loadTexture3D(w, w, w, nullptr, config);
    gl::clearTexImage(texture, 0, GL_RGBA, GL_FLOAT, nullptr);

    return texture;
}


GLuint
idk::VXGI::allocateAlbedoTexture( size_t w )
{
    static constexpr idk::glTextureConfig config = {
        .target         = GL_TEXTURE_3D,
        .internalformat = GL_RGBA8,
        .format         = GL_RGBA,
        .minfilter      = GL_NEAREST,
        .magfilter      = GL_NEAREST,
        .datatype       = GL_UNSIGNED_BYTE,
        .genmipmap      = GL_FALSE
    };

    GLuint texture = gltools::loadTexture3D(w, w, w, nullptr, config);
    gl::clearTexImage(texture, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    return texture;
}


GLuint
idk::VXGI::allocateNormalTexture( size_t w )
{
    static constexpr idk::glTextureConfig config = {
        .target         = GL_TEXTURE_3D,
        .internalformat = GL_RGBA8,
        .format         = GL_RGBA,
        .minfilter      = GL_NEAREST,
        .magfilter      = GL_NEAREST,
        .datatype       = GL_UNSIGNED_BYTE,
        .genmipmap      = GL_FALSE
    };

    GLuint texture = gltools::loadTexture3D(w, w, w, nullptr, config);
    gl::clearTexImage(texture, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    return texture;
}


static glm::mat4 light_matrix;

void
idk::VXGI::shadowPass( idk::glFramebuffer &buffer_out, idk::Camera &camera, glm::vec3 light_dir,
                       idk::glShaderProgram &program, GLuint draw_indirect_buffer,
                       const std::vector<idk::glDrawCmd> &commands )
{
    gl::disable(GL_CULL_FACE);

    buffer_out.bind();
    buffer_out.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    static const float     B = VXGI_WORLD_HALF_BOUNDS;
    static const glm::mat4 P = glm::ortho(-B, B, -B, B, -B, B);

    glm::mat4 view = glm::lookAt(
        - light_dir,
        glm::vec3(0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    light_matrix = P * view;

    program.bind();
    program.set_mat4("un_P", light_matrix);

    gl::multiDrawElementsIndirect(
        GL_TRIANGLES,
        GL_UNSIGNED_INT,
        nullptr,
        commands.size(),
        sizeof(idk::glDrawCmd)
    );

    gl::enable(GL_CULL_FACE);
}



void
idk::VXGI::renderTexture( idk::glFramebuffer &buffer_out, idk::Camera &camera, idk::glShaderProgram &program,
                           GLuint draw_indirect_buffer, const std::vector<idk::glDrawCmd> &commands,
                           idk::glDepthCascade &depthcascade )
{
    static const float B     = VXGI_WORLD_HALF_BOUNDS;
    static const float theta = glm::radians(90.0f);

    static const glm::mat4 ident = glm::mat4(1.0f);
    static const glm::mat4 projection_matrices[3] = {
        glm::ortho(-B, B, -B, B, -B, B) * ident,
        glm::ortho(-B, B, -B, B, -B, B) * glm::rotate(ident, theta, glm::vec3(0.0f, -1.0f, 0.0f)),
        glm::ortho(-B, B, -B, B, -B, B) * glm::rotate(ident, theta, glm::vec3(-1.0f, 0.0f, 0.0f))
    };

    gl::disable(GL_DEPTH_TEST, GL_CULL_FACE);

    buffer_out.bind();
    program.bind();

    for (int i=0; i<3; i++)
    {
        const glm::mat4 &P = projection_matrices[i];
        program.set_mat4("un_P", P);

        gl::multiDrawElementsIndirect(
            GL_TRIANGLES,
            GL_UNSIGNED_INT,
            nullptr,
            commands.size(),
            sizeof(idk::glDrawCmd)
        );
    }

    gl::enable(GL_DEPTH_TEST, GL_CULL_FACE);
}


void
idk::VXGI::injectRadiance( idk::glShaderProgram &program, idk::Camera &camera, idk::glFramebuffer &buffer_out,
                           glm::vec3 light_dir, idk::glDepthCascade &depthcascade )
{
    program.bind();

    program.set_vec4("un_cascade_depths", depthcascade.getCascadeDepths(camera.far));
    program.set_sampler2DArray("un_dirlight_depthmap", depthcascade.getTextureArray());
    program.set_sampler2D("un_depthmap", buffer_out.depth_attachment);
    program.set_mat4("un_light_matrix", light_matrix);

    program.dispatch(VXGI_TEXTURE_SIZE/4);

    gl::memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}


// void
// idk::VXGI::generateMipmap( idk::glShaderProgram &program, GLuint textures[6] )
// {
//     program.bind();

//     GLuint size  = GLuint(VXGI_TEXTURE_SIZE) / 2;
//     GLuint level = 1;

//     while (size >= 1)
//     {

//         for (int i=0; i<6; i++)
//         {
//             gl::bindImageTexture(20+i, textures[i], level,   GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
//             gl::bindImageTexture(26+i, textures[i], level-1, GL_TRUE, 0, GL_READ_ONLY,  GL_RGBA16F);
//         }

//         program.dispatch(size/1);

//         size /= 2;
//         level += 1;

//         gl::memoryBarrier(GL_ALL_BARRIER_BITS);
//     }

// }


void
idk::VXGI::generateMipmap( idk::glShaderProgram &program1, idk::glShaderProgram &program2, GLuint textures[6] )
{
    // for (int i=0; i<6; i++)
    // {
    //     gl::generateTextureMipmap(textures[i]);
    // }

    // return;

    program1.bind();

    GLuint size  = GLuint(VXGI_TEXTURE_SIZE) / 2;
    GLuint level = 1;

    while (size >= 8)
    {
        for (int i=0; i<6; i++)
        {
            gl::bindImageTexture(20+i, textures[i], level,   GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
            gl::bindImageTexture(26+i, textures[i], level-1, GL_TRUE, 0, GL_READ_ONLY,  GL_RGBA16F);
        }

        program1.dispatch(size/8);

        size /= 2;
        level += 1;

        gl::memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }


    program2.bind();

    while (size >= 1)
    {
        for (int i=0; i<6; i++)
        {
            gl::bindImageTexture(20+i, textures[i], level,   GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
            gl::bindImageTexture(26+i, textures[i], level-1, GL_TRUE, 0, GL_READ_ONLY,  GL_RGBA16F);
        }

        program2.dispatch(size);

        size /= 2;
        level += 1;

        gl::memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }
}
