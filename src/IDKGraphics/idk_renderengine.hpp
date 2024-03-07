#pragma once

#include "render/idk_sdl_glew_init.hpp"

#include "batching/idk_model_allocator.hpp"
#include "batching/idk_render_queue.hpp"

#include <libidk/idk_gl.hpp>
#include <libidk/idk_allocator.hpp>

#include "storage/idk_ubo_general.hpp"

#include "camera/idk_camera.hpp"
#include "lighting/IDKlighting.hpp"

#include <unordered_map>

#define IDK_MAX_POINTLIGHTS 10
#define IDK_MAX_SPOTLIGHTS 10
#define IDK_MAX_DIRLIGHTS 10


namespace idk
{
    class  RenderEngine;

    enum class RenderSetting: uint32_t
    {
        VXGI                   = 1 << 0,
        VXGI_DEBUG             = 1 << 1,
        VXGI_LIVE_VOXELIZATION = 1 << 2,
        VXGI_LIVE_INJECTION    = 1 << 3,
        VXGI_LIVE_PROPAGATION  = 1 << 4,
        VXGI_DIFFUSE           = 1 << 5,
        VXGI_SPECULAR          = 1 << 6,

        POST_PROCESSING        = 1 << 7,
        CHROMATIC_ABERRATION   = 1 << 8,
        BLOOM                  = 1 << 9,
        MOTION_BLUR            = 1 << 10
    };

};



class IDK_VISIBLE idk::RenderEngine
{
private:
    internal::SDL2_WindowSystem         m_windowsys;
    glm::ivec2                          m_resolution;

    uint32_t                            m_render_settings = ~0;

    // idk::glFramebuffers ------------------------------------
    static const size_t                 NUM_SCRATCH_BUFFERS    = 8;
    static const size_t                 ATTACHMENTS_PER_BUFFER = 1;


    glFramebuffer                       m_scratchbuffers[4];

    glFramebuffer                       m_mainbuffer_0;
    glFramebuffer                       m_mainbuffer_1;
    glFramebuffer                       m_finalbuffer;

    glFramebuffer                       m_geom_buffer;
    glFramebuffer                       m_volumetrics_buffer;


    static constexpr int                BLOOM_MAX_LEVEL = 6;
    glFramebuffer                       m_bloom_buffers[BLOOM_MAX_LEVEL+1];
    GLuint                              m_velocitybuffer;
    GLuint                              m_positionbuffer;
    // -----------------------------------------------------------------------------------------

    // Shaders
    // -----------------------------------------------------------------------------------------
    idk::Allocator<glShaderProgram>         m_programs;
    std::map<std::string, int>              m_program_ids;
    // -----------------------------------------------------------------------------------------

    // UBO
    // -----------------------------------------------------------------------------------------
    using UBO_type = idk::glTemplatedBufferObject<GL_UNIFORM_BUFFER, idk::UBORenderData>;
    idk::UBORenderData                  m_RenderData;
    UBO_type                            m_UBO_RenderData;

    glUBO                               m_UBO_dirlights;
    // -----------------------------------------------------------------------------------------


    int                                 m_active_camera_id;
    idk::Allocator<Camera>              m_camera_allocator;
    idk::LightSystem                    m_lightsystem;
    idk::ModelAllocator                 m_model_allocator;

    idk::Allocator<idk::RenderQueue>    m_render_queues;
    idk::Allocator<idk::RenderQueue>    m_user_render_queues;
    int                                 m_RQ, m_viewspace_RQ, m_shadow_RQ, m_GI_RQ;


    using SSBO_type = idk::glTemplatedBufferObject<GL_SHADER_STORAGE_BUFFER, idk::DrawIndirectData>;

    idk::glBufferObject<GL_DRAW_INDIRECT_BUFFER>   m_DrawCommandBuffer;
    idk::DrawIndirectData                         *m_DrawIndirectData;
    SSBO_type                                      m_DrawIndirectSSBO;

    // // VXGI
    // // -----------------------------------------------------------------------------------------
    // GLuint                              vxgi_radiance[6];
    // GLuint                              vxgi_radiance_2[6];
    // GLuint                              vxgi_albedo;
    // GLuint                              vxgi_normal;
    // // -----------------------------------------------------------------------------------------

    // Initialization
    // -----------------------------------------------------------------------------------------
    void                                init_screenquad();
    void                                init_framebuffers( int width, int height );
    void                                init_all( std::string name, int w, int h );
    // -----------------------------------------------------------------------------------------


    // Light sources
    // -----------------------------------------------------------------------------------------
    idk::Allocator<IDK_Dirlight>        m_dirlights;
    idk::Allocator<IDK_Pointlight>      m_pointlights;
    idk::Allocator<IDK_Spotlight>       m_spotlights;
    idk::Allocator<IDK_Atmosphere>      m_atmospheres;
    GLuint                              m_skybox;

    int                                 m_unit_line;
    int                                 m_unit_sphere;
    int                                 m_unit_cone;
    // -----------------------------------------------------------------------------------------



    void                                update_UBO_camera();
    void                                update_UBO_dirlights( idk::UBORenderData &data );


    void                                updateLightsourcesUBO( idk::UBORenderData &data );
    void                                updateAtmosphereUBO( idk::UBORenderData &data );

    idk::glDrawCmd                      genDirlightDrawCommand   ( idk::ModelAllocator & );
    idk::glDrawCmd                      genLightsourceDrawCommand( int, uint32_t, idk::ModelAllocator & );
    idk::glDrawCmd                      genAtmosphereDrawCommand( idk::ModelAllocator & );


    void                                shadowpass_dirlights();
    void                                shadowpass_pointlights();
    void                                shadowpass_spotlights();
    void                                shadowpass();
    // -----------------------------------------------------------------------------------------


    idk::RenderQueue &                  _getRenderQueue( int id ) { return m_render_queues.get(id); };


    // Render stages    
    // ------------------------------------------------------------------------------------
    void RenderStage_geometry( idk::Camera &, float dtime,
                               glFramebuffer &buffer_out );


    void RenderStage_volumetrics( idk::Camera &,
                                  glFramebuffer &buffer_in,
                                  glFramebuffer &buffer_out );

    void RenderStage_atmospheres( idk::Camera &,
                                  glFramebuffer &buffer_in,
                                  glFramebuffer &buffer_out );


    void RenderStage_pointlights();
    void RenderStage_spotlights();


    void RenderStage_lighting( idk::Camera &, float dtime,
                               glFramebuffer &buffer_in,
                               glFramebuffer &buffer_out );


    void PostProcess_bloom( glFramebuffer &buffer_in,
                            glFramebuffer &buffer_out );

    void PostProcess_chromatic_aberration( glFramebuffer &buffer_in,
                                           glFramebuffer &buffer_out );

    void PostProcess_SSR();

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

    const static uint32_t                   ARMATURE_MAX_BONES = 70;

    GLuint                                  m_quad_VAO, m_quad_VBO;
    GLuint                                  solid_shader;

    std::vector<GLuint>                     skyboxes;
    std::vector<std::pair<GLuint, GLuint>>  skyboxes_IBL;
    int                                     current_skybox = 0;
    GLuint                                  BRDF_LUT;


    void                                compileShaders();

    SDL_Window *                        getWindow()    { return m_windowsys.getMainWindow(); };
    SDL_GLContext                       getGLContext() { return m_windowsys.getGlContext();  };

    void                                setRenderSetting( idk::RenderSetting, bool );
    bool                                getRenderSetting( idk::RenderSetting );


    int                                 createCamera();
    void                                useCamera( int cam_id ) { m_active_camera_id = cam_id; };
    idk::Camera &                       getCamera( int cam_id ) { return m_camera_allocator.get(cam_id); };
    idk::Camera &                       getCamera(            ) { return getCamera(m_active_camera_id);  };
    int                                 activeCamera() { return m_active_camera_id; };
    idk::Allocator<Camera> &            getCameras() { return m_camera_allocator; };


                                        IDK_ALLOCATOR_ACCESS(Dirlight,   IDK_Dirlight,   m_dirlights);
                                        IDK_ALLOCATOR_ACCESS(Pointlight, IDK_Pointlight, m_pointlights);
                                        IDK_ALLOCATOR_ACCESS(Spotlight,  IDK_Spotlight,  m_spotlights);
                                        IDK_ALLOCATOR_ACCESS(Atmosphere, IDK_Atmosphere, m_atmospheres);

    int                                 createRenderQueue( const std::string &program );
    void                                destroyRenderQueue( int RQ );


    idk::LightSystem &                  lightSystem() { return m_lightsystem; };
    ModelAllocator &                    modelAllocator() { return m_model_allocator; };


    void getVertices( int model_id, size_t &num_vertices, std::unique_ptr<idk::Vertex_P_N_T_UV[]> &vertices )
    {
        m_model_allocator.getVertices(model_id, num_vertices, vertices);
    };

    void getIndices( int model_id, size_t &num_indices, std::unique_ptr<uint32_t[]> &indices )
    {
        m_model_allocator.getIndices(model_id, num_indices, indices);
    };


    int                                 loadSkybox( const std::string &filepath );
    void                                useSkybox( int skybox ) { current_skybox = skybox; };



    int                                 loadModel( const std::string &filepath );

    void                                drawLine( const glm::vec3 A, const glm::vec3 B, float thickness );

    void                                drawModel( int model, const glm::mat4 & );
    void                                drawModelRQ( int RQ, int model, const glm::mat4 & );

    void                                drawModelViewspace( int model, const glm::mat4 & );
    void                                drawShadowCaster( int model, const glm::mat4 & );

    /** Draw a model which contributes to global illumination. */
    void                                drawEnvironmental( int model, const glm::mat4 & );


    int                                 createProgram( const std::string &name,
                                                       const std::string &root,
                                                       const std::string &vs,
                                                       const std::string &fs );


    int                                 createProgram( const std::string &name,
                                                       const idk::glShaderProgram &program );



    idk::glShaderProgram &getProgram( const std::string &name )
    {
        IDK_ASSERT("No such program", m_program_ids.contains(name));

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


