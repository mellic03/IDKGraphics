#pragma once

#include <libidk/GL/idk_glFramebuffer.hpp>
#include "../lighting/idk_shadowcascade.hpp"
#include "idk_renderqueue.hpp"
#include "../camera/idk_camera.hpp"


#define VXGI_TEXTURE_FORMAT     GL_RGBA16F
#define VXGI_TEXTURE_SIZE       256.0f
#define VXGI_WORLD_BOUNDS       32.0f
#define VXGI_WORLD_HALF_BOUNDS  (VXGI_WORLD_BOUNDS / 2.0f)


namespace idk::VXGI
{
    GLuint  allocateTexture( size_t w );

    GLuint  allocateRadianceTexture( size_t w );


    void    shadowPass( idk::glFramebuffer &buffer_out, idk::Camera &camera, glm::vec3 light_dir,
                        idk::glShaderProgram &program, idk::RenderQueue &RQ,
                        idk::ModelSystem &MS );

    void    renderTexture( idk::glFramebuffer &buffer_out, idk::Camera &camera, idk::glShaderProgram &program,
                           idk::RenderQueue &RQ, GLuint albedo, GLuint normal, idk::ModelSystem &MS,
                           idk::glDepthCascade &depthcascade );

    void    injectRadiance( idk::glShaderProgram &program, idk::Camera &camera, idk::glFramebuffer &buffer_out,
                           glm::vec3 light_dir, idk::glDepthCascade &depthcascade );


    void    generateMipmap( idk::glShaderProgram &program, GLuint texture );

};

