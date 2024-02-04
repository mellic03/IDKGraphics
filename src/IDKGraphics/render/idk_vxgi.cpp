#include "idk_vxgi.hpp"


static void
drawmethod( idk::glShaderProgram &program, int model_id,
            const glm::mat4 &transform, idk::ModelSystem &MS )
{
    idk::Model &model = MS.getModel(model_id);
    program.set_mat4("un_model", transform);

    idk::gl::bindVertexArray(model.VAO);
    uint64_t start_idx = 0;

    for (idk::Mesh &mesh: model.meshes)
    {
        auto &material = MS.getMaterial(mesh.material_id);
        program.set_int("un_material_id", material.bindless_idx);

        idk::gl::drawElements(
            GL_TRIANGLES, mesh.num_indices, GL_UNSIGNED_INT,
            (void *)(start_idx)
        );

        start_idx += mesh.num_indices * sizeof(GLuint);
    }
}


static void
VXGI_clearTexture( GLuint texture )
{
    IDK_GLCALL( glClearTexImage(texture, 0, GL_RGBA, GL_UNSIGNED_INT, nullptr); )
}


GLuint
idk::VXGI::allocateTexture( size_t w )
{
    #if VXGI_TEXTURE_FORMAT == GL_RGBA16F
        static constexpr idk::glTextureConfig config = {
            .target         = GL_TEXTURE_3D,
            .internalformat = GL_RGBA16F,
            .format         = GL_RGBA,
            .minfilter      = GL_LINEAR_MIPMAP_LINEAR,
            .magfilter      = GL_NEAREST,
            .datatype       = GL_HALF_FLOAT,
            .genmipmap      = GL_TRUE,
        };
    #else
        static constexpr idk::glTextureConfig config = {
            .target         = GL_TEXTURE_3D,
            .internalformat = GL_RGBA8,
            .format         = GL_RGBA,
            .minfilter      = GL_LINEAR_MIPMAP_LINEAR,
            .magfilter      = GL_NEAREST,
            .datatype       = GL_UNSIGNED_INT,
            .genmipmap      = GL_TRUE,
        };
    #endif

    GLuint texture = gltools::genTexture3D(w, w, w, config);
    IDK_GLCALL( glClearTexImage(texture, 0, GL_RGBA, GL_HALF_FLOAT, nullptr); )

    return texture;
}

GLuint
idk::VXGI::allocateRadianceTexture( size_t w )
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

    return gltools::genTexture3D(w, w, w, config);
}



static glm::mat4 ligh_matrix;


void
idk::VXGI::shadowPass( idk::glFramebuffer &buffer_out, idk::Camera &camera, glm::vec3 light_dir,
                       idk::glShaderProgram &program, idk::RenderQueue &RQ,
                       idk::ModelSystem &MS )
{
    // gl::enable(GL_DEPTH_TEST);
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

    ligh_matrix = P * view;

    program.bind();
    program.set_mat4("un_P", ligh_matrix);

    for (auto &[model, dummy, transform]: RQ)
    {
        idk::drawmethods::draw_untextured(
            program,
            model,
            transform,
            MS
        );
    }

    gl::enable(GL_CULL_FACE);
}



void
idk::VXGI::renderTexture( idk::glFramebuffer &buffer_out, idk::Camera &camera, idk::glShaderProgram &program,
                          idk::RenderQueue &RQ, GLuint albedo, GLuint normal, idk::ModelSystem &MS,
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

    gl::disable(GL_DEPTH_TEST, GL_CULL_FACE, GL_BLEND);

    buffer_out.bind();
    program.bind();

    program.set_vec4("un_cascade_depths", depthcascade.getCascadeDepths(camera.farPlane()));
    program.set_sampler2DArray("un_dirlight_depthmap", depthcascade.getTextureArray());
    program.set_sampler2D("un_depthmap", buffer_out.depth_attachment);
    program.set_mat4("un_light_matrix", ligh_matrix);

    for (int i=0; i<3; i++)
    {
        const glm::mat4 &P = projection_matrices[i];
        program.set_mat4("un_P", P);

        for (auto &[model, dummy, transform]: RQ)
        {
            drawmethod(program, model, transform, MS);
        }
    }

    gl::enable(GL_DEPTH_TEST, GL_CULL_FACE);
}


void
idk::VXGI::injectRadiance( idk::glShaderProgram &program, idk::Camera &camera, idk::glFramebuffer &buffer_out,
                           glm::vec3 light_dir,
                           idk::glDepthCascade &depthcascade )
{
    program.bind();
    program.set_vec3("un_light_dir", light_dir);

    program.set_vec4("un_cascade_depths", depthcascade.getCascadeDepths(camera.farPlane()));
    program.set_sampler2DArray("un_dirlight_depthmap", depthcascade.getTextureArray());
    program.set_sampler2D("un_depthmap", buffer_out.depth_attachment);
    program.set_mat4("un_light_matrix", ligh_matrix);


    static constinit GLuint size = GLuint(VXGI_TEXTURE_SIZE);
    gl::dispatchCompute(size/4, size/4, size/4);
    gl::memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    program.unbind();
}


void
idk::VXGI::generateMipmap( idk::glShaderProgram &program, GLuint texture )
{
    program.bind();

    GLuint size  = GLuint(VXGI_TEXTURE_SIZE) / 2;
    GLuint level = 1;

    while (size >= 4)
    {
        gl::bindImageTexture(5, texture, level,   GL_TRUE, 0, GL_WRITE_ONLY, VXGI_TEXTURE_FORMAT);
        gl::bindImageTexture(6, texture, level-1, GL_TRUE, 0, GL_READ_ONLY,  VXGI_TEXTURE_FORMAT);

        gl::dispatchCompute(size/4, size/4, size/4);

        size /= 2;
        level += 1;

        gl::memoryBarrier(GL_ALL_BARRIER_BITS);
    }

}
