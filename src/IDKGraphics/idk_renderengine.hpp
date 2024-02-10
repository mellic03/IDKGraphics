#pragma once

#include "render/idk_sdl_glew_init.hpp"
#include "render/idk_renderqueue.hpp"

#include "batching/idk_model_allocator.hpp"

#include <libidk/GL/common.hpp>
#include <libidk/idk_allocator.hpp>

#include "camera/idk_camera.hpp"
#include "idk_noisegen.hpp"
#include "lighting/IDKlighting.hpp"

#include <unordered_map>

#define IDK_MAX_POINTLIGHTS 10
#define IDK_MAX_SPOTLIGHTS 10
#define IDK_MAX_DIRLIGHTS 10


namespace idk { class RenderEngine; };




class idk_RenderData
{
    
};




class IDK_VISIBLE idk::RenderEngine
{
private:
    internal::SDL2_WindowSystem         m_windowsys;
    glm::ivec2                          m_resolution;

    // idk::glFramebuffers ------------------------------------
    static const size_t                 NUM_SCRATCH_BUFFERS    = 8;
    static const size_t                 ATTACHMENTS_PER_BUFFER = 1;

    std::vector<glFramebuffer>          m_scratchbufs0;
    std::vector<glFramebuffer>          m_scratchbufs1;
    std::vector<glFramebuffer>          m_scratchbufs2;
    std::vector<glFramebuffer>          m_scratchbufs3;

    glFramebuffer                       m_mainbuffer_0;
    glFramebuffer                       m_mainbuffer_1;
    glFramebuffer                       m_finalbuffer;

    glFramebuffer                       m_geom_buffer;
    glFramebuffer                       m_volumetrics_buffer;

    // -----------------------------------------------------------------------------------------

    // Shaders
    // -----------------------------------------------------------------------------------------
    idk::Allocator<glShaderProgram>         m_programs;
    std::map<std::string, int>              m_program_ids;
    // -----------------------------------------------------------------------------------------

    // UBO
    // -----------------------------------------------------------------------------------------
    glUBO                               m_UBO_pointlights;
    glUBO                               m_UBO_dirlights;
    glUBO                               m_UBO_camera;
    glUBO                               m_UBO_armature;
    // -----------------------------------------------------------------------------------------


    int                                 m_active_camera_id;
    Allocator<Camera>                   m_camera_allocator;
    idk::ModelSystem                    m_modelsystem;
    idk::LightSystem                    m_lightsystem;
    idk::ModelAllocator                 m_model_allocator;

    // Render queues
    // -----------------------------------------------------------------------------------------
    idk::Allocator<idk::RenderQueue>    m_private_RQs;
    idk::Allocator<idk::RenderQueue>    m_public_RQs;

    int                                 m_RQ;

    GLuint                              vxgi_normal;
    GLuint                              vxgi_albedo;
    GLuint                              vxgi_radiance[6];
    GLuint                              vxgi_propagation[6];

    glSSBO                              vxgi_SSBO;
    idk::RenderQueue                    m_vxgi_RQ;
    idk::RenderQueue                    m_shadow_render_queue; 
    // -----------------------------------------------------------------------------------------

    // Initialization
    // -----------------------------------------------------------------------------------------
    void                                init_screenquad();
    void                                init_framebuffers( int width, int height );
    void                                init_all( std::string name, int w, int h );
    // -----------------------------------------------------------------------------------------

    // Draw-related methods
    // -----------------------------------------------------------------------------------------
    void                                update_UBO_camera();
    void                                update_UBO_dirlights();
    void                                update_UBO_pointlights();
    void                                shadowpass_dirlights();
    // -----------------------------------------------------------------------------------------


    idk::RenderQueue &                  _getRenderQueue( int id );

    int                                 _createRenderQueue( const std::string &program_name,
                                                            const idk_drawmethod & );


    // Render stages    
    // ------------------------------------------------------------------------------------
    void RenderStage_geometry( idk::Camera &, float dtime,
                               glFramebuffer &buffer_out );


    void RenderStage_volumetrics( idk::Camera &,
                                  glFramebuffer &buffer_in,
                                  glFramebuffer &buffer_out );


    void RenderStage_lighting( idk::Camera &, float dtime,
                               glFramebuffer &buffer_in,
                               glFramebuffer &buffer_out );


    void PostProcess_bloom( glFramebuffer &buffer_in,
                            glFramebuffer &buffer_out );

    void PostProcess_chromatic_aberration( glFramebuffer &buffer_in,
                                           glFramebuffer &buffer_out );

    void PostProcess_colorgrading( idk::Camera &,
                                   glFramebuffer &buffer_in,
                                   glFramebuffer &buffer_out );


    void RenderStage_postprocessing( idk::Camera &,
                                     glFramebuffer &buffer_in,
                                     glFramebuffer &buffer_out );
    // ------------------------------------------------------------------------------------



    /** Run a shader on the output textures of "in" and render the result to the default frame buffer.s
    */
    static void    f_fbfb( glShaderProgram &, glFramebuffer &buffer_in );

    static void    tex2tex( glShaderProgram &, glFramebuffer &a,
                            glFramebuffer &b, glFramebuffer &out );
    // ------------------------------------------------------------------------------------




public:
    bool                                m_vxgi_debug = false;
    glFramebuffer                       m_vxgi_buffer;

    RenderEngine( const std::string &name, int w, int h,
                  int gl_major, int gl_minor,
                  uint32_t flags=0 );

    RenderEngine( const idk::RenderEngine & ) = delete;



    static void    tex2tex ( glShaderProgram &, glFramebuffer &in, glFramebuffer &out );

    const static uint32_t               ARMATURE_MAX_BONES = 70;

    GLuint                              m_quad_VAO, m_quad_VBO;
    GLuint                              solid_shader;

    std::vector<GLuint>                     skyboxes;
    std::vector<std::pair<GLuint, GLuint>>  skyboxes_IBL;
    int                                     current_skybox = 0;
    GLuint                                  BRDF_LUT;


    void                                compileShaders();

    SDL_Window *                        getWindow()    { return m_windowsys.getMainWindow(); };
    SDL_GLContext                       getGLContext() { return m_windowsys.getGlContext();  };



    int                                 createCamera();
    void                                useCamera( int cam_id ) { m_active_camera_id = cam_id; };
    idk::Camera &                       getCamera( int cam_id ) { return m_camera_allocator.get(cam_id); };
    idk::Camera &                       getCamera(            ) { return getCamera(m_active_camera_id);  };
    int                                 activeCamera() { return m_active_camera_id; };
    idk::Allocator<Camera> &            getCameras() { return m_camera_allocator; };


    idk::LightSystem &                  lightSystem() { return m_lightsystem; };
    ModelSystem &                       modelSystem() { return m_modelsystem; };
    ModelAllocator &                    modelAllocator() { return m_model_allocator; };

    int                                 loadSkybox( const std::string &filepath );
    void                                useSkybox( int skybox ) { current_skybox = skybox; };

    int                                 createRenderQueue( const std::string &program_name,
                                                           const RenderQueueConfig &,
                                                           const idk_drawmethod & );

    int                                 createRenderQueue( int program,
                                                           const RenderQueueConfig &,
                                                           const idk_drawmethod & );

    idk::RenderQueue &                  getRenderQueue( int id );

    void                                drawModelRQ( int rq, int model, const glm::mat4 & );
    void                                drawModel( int model, const glm::mat4 & );
    void                                drawShadowCaster( int model, const glm::mat4 & );

    /** Draw a model which will contribute to global illumination. */
    void                                drawEnvironmental( int model, const glm::mat4 & );


    int                                 createProgram( const std::string &name,
                                                       const std::string &root,
                                                       const std::string &vs,
                                                       const std::string &fs );


    int                                 createProgram( const std::string &name,
                                                       const idk::glShaderProgram &program );

    // template <typename ...Args>
    // int                                 createProgram( const std::string &name,
    //                                                    idk::glShaderStage first,
    //                                                    Args... rest );


    idk::glShaderProgram &getProgram( const std::string &name )
    {
        int id = m_program_ids[name];
        return m_programs.get(id);
    };

    idk::glShaderProgram &getProgram( int id )
    {
        return m_programs.get(id);
    };

    const std::map<std::string, int> &getProgramIDs()
    {
        return m_program_ids;
    }


    void                                beginFrame();
    void                                endFrame( float dt );
    void                                swapWindow();
    void                                resize( int w, int h );

    GLuint                              getFinalImage() { return m_finalbuffer.attachments[0]; };

    glm::ivec2                          resolution() const { return m_resolution;   };
    int                                 width()      const { return m_resolution.x; };
    int                                 height()     const { return m_resolution.y; };

};




// template <typename ...Args>
// int
// idk::RenderEngine::createProgram( const std::string &name, idk::glShaderStage first, Args... rest )
// {
//     int id = m_programs.create(idk::glShaderProgram(rest...));
//     m_program_ids[name] = id;

//     return id;
// }


