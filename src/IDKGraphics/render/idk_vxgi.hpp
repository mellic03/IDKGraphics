#pragma once

#include <libidk/GL/idk_glFramebuffer.hpp>
#include <libidk/GL/idk_glShaderProgram.hpp>
#include <libidk/GL/idk_glDrawCommand.hpp>

#include "../lighting/idk_shadowcascade.hpp"
#include "../camera/idk_camera.hpp"


#define VXGI_TEXTURE_FORMAT     GL_RGBA16F
#define VXGI_TEXTURE_SIZE       64.0f
#define VXGI_WORLD_BOUNDS       16.0f
#define VXGI_WORLD_HALF_BOUNDS  (VXGI_WORLD_BOUNDS / 2.0f)


namespace idk::VXGI
{
    GLuint  allocateTexture( size_t w );
    GLuint  allocateAlbedoTexture( size_t w );
    GLuint  allocateNormalTexture( size_t w );

    void    shadowPass( idk::glFramebuffer &buffer_out, IDK_Camera &camera, glm::vec3 light_dir,
                        idk::glShaderProgram &program, GLuint draw_indirect_buffer,
                        const std::vector<idk::glDrawCmd> &commands );

    void    renderTexture( idk::glFramebuffer &buffer_out, IDK_Camera &camera, idk::glShaderProgram &program,
                           GLuint draw_indirect_buffer, const std::vector<idk::glDrawCmd> &commands,
                           idk::glDepthCascade &depthcascade );

    void    injectRadiance( idk::glShaderProgram &program, IDK_Camera &camera, idk::glFramebuffer &buffer_out,
                           glm::vec3 light_dir, idk::glDepthCascade &depthcascade );


    // void    generateMipmap( idk::glShaderProgram &program, GLuint textures[6] );
    void    generateMipmap( idk::glShaderProgram &program, idk::glShaderProgram &program2, GLuint textures[6] );

};

