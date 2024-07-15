#pragma once

#include <libidk/idk_sdl_glew_init.hpp>
#include <libidk/idk_log.hpp>
#include <libidk/idk_gl.hpp>
#include <libidk/idk_allocator.hpp>

#include "batching/idk_model_allocator.hpp"
#include "batching/idk_render_queue.hpp"

// #include "storage/idk_ubo_general.hpp"
#include "storage/buffers.hpp"

#include "idk_overlay.hpp"
#include "render/particle.hpp"

#include "camera/idk_camera.hpp"
#include "lighting/IDKlighting.hpp"

#include <unordered_map>
#include <queue>


// Forward declarations
namespace idk
{
    struct Transform;
    struct RenderSettings;
};


namespace idk
{
    struct SSAO_Settings
    {
        bool  enabled   = true;
        float factor    = 2.0f;
        float intensity = 0.4f;

        int   samples   = 9;
        float radius    = 0.85f;
        float bias      = -0.02f;
    };

    class RenderEngine;

    enum class RenderSetting: uint32_t
    {
        VXGI                   = 1 << 0,
        VXGI_DEBUG             = 1 << 1,
        VXGI_LIVE_VOXELIZATION = 1 << 2,
        VXGI_LIVE_INJECTION    = 1 << 3,
        VXGI_LIVE_PROPAGATION  = 1 << 4,
        VXGI_DIFFUSE           = 1 << 5,
        VXGI_SPECULAR          = 1 << 6,

        LIGHTPROBE_VIS         = 1 << 7,

        POST_PROCESSING        = 1 << 8,
        CHROMATIC_ABERRATION   = 1 << 9,
        BLOOM                  = 1 << 10,
        MOTION_BLUR            = 1 << 11
    };

};



class IDK_VISIBLE idk::RenderEngine
{
private:

    struct TextQuad
    {
        float x, y, w, h;
        float radius;
        glm::vec4 color;
        uint32_t texture;
    };

    internal::SDL2_WindowSystem         m_windowsys;
    glm::ivec2                          m_resolution;
    uint32_t                            m_render_settings = ~0;
    RenderSettings *                    m_rendersettings;

    SDL_Surface *                       m_textsurface;

    std::queue<RenderOverlay>           m_overlays;
    std::queue<RenderOverlayFill>       m_overlayfills;
    std::queue<uint32_t>                m_texture_overlays;
    idk::Allocator<ParticleEmitter>     m_particle_emitters;


    // idk::glFramebuffers
    // -----------------------------------------------------------------------------------------
    static const size_t                 NUM_SCRATCH_BUFFERS    = 8;
    static const size_t                 ATTACHMENTS_PER_BUFFER = 1;

    // glFramebuffer                       m_scratchbuffers[4];
    glFramebuffer                       m_scratchbuffers2[2];

    static constexpr int                BLOOM_MAX_LEVEL = 6;
    glFramebuffer                       m_bloom_buffers[BLOOM_MAX_LEVEL+1];

    glFramebuffer                       m_envprobe_buffer;
    glFramebuffer                       m_convolve_buffer;
    glFramebuffer                       m_lightprobe_buffer;
    // -----------------------------------------------------------------------------------------

    // Shaders
    // -----------------------------------------------------------------------------------------
    idk::Allocator<glShaderProgram>                 m_programs;
    std::map<std::string, int>                      m_program_ids;
    // -----------------------------------------------------------------------------------------

    // Shader buffer objects
    // -----------------------------------------------------------------------------------------
    idk::glBufferObject<GL_UNIFORM_BUFFER>          m_UBO;
    idk::UBO_Buffer                                 m_UBO_buffer;

    idk::glBufferObject<GL_SHADER_STORAGE_BUFFER>   m_SSBO;
    idk::SSBO_Buffer                                m_SSBO_buffer;

    idk::glBufferObject<GL_DRAW_INDIRECT_BUFFER>    m_DIB;
    std::vector<idk::glDrawCmd>                     m_DIB_buffer;

    idk::glBufferObject<GL_DRAW_INDIRECT_BUFFER>    m_lightsource_DIB;
    // -----------------------------------------------------------------------------------------


    int                                 m_active_camera_id;
    idk::Allocator<IDK_Camera>          m_cameras;
    idk::ModelAllocator                 m_model_allocator;

    idk::Allocator<idk::RenderQueue>    m_render_queues;
    idk::Allocator<idk::RenderQueue>    m_user_render_queues;
    idk::Allocator<idk::RenderQueue>    m_user_shadow_queues;
    int                                 m_RQ, m_viewspace_RQ, m_shadow_RQ, m_GI_RQ;
    int                                 m_decal_RQ;

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

    void                                _gen_envprobes();
    // -----------------------------------------------------------------------------------------


    // Light sources
    // -----------------------------------------------------------------------------------------
    idk::Allocator<IDK_Dirlight>        m_dirlights;
    idk::Allocator<IDK_Pointlight>      m_pointlights;
    idk::Allocator<IDK_Spotlight>       m_spotlights;
    size_t                              m_light_cmd_offsets[3];

    // idk::Allocator<IDK_Atmosphere>      m_atmospheres;

    int                                 m_unit_line;
    int                                 m_unit_cube;
    int                                 m_unit_sphere;
    int                                 m_unit_sphere_FF;
    int                                 m_unit_cone;
    int                                 m_unit_cylinder_FF;
    // -----------------------------------------------------------------------------------------



    void                                update_UBO_camera( idk::UBO_Buffer& );
    void                                update_UBO_lightsources( idk::UBO_Buffer& );

    idk::glDrawCmd                      genDirlightDrawCommand   ( idk::ModelAllocator & );
    idk::glDrawCmd                      genLightsourceDrawCommand( int, uint32_t, idk::ModelAllocator & );
    // idk::glDrawCmd                      genAtmosphereDrawCommand( idk::ModelAllocator & );


    void                                shadowpass_dirlights();
    void                                shadowpass_pointlights();
    void                                shadowpass_spotlights();

    void                                _render_cubemap( const glm::vec3&, uint32_t,
                                                         idk::glShaderProgram&,
                                                         idk::RenderQueue&, idk::glFramebuffer& );

    void                                _render_envmap( const glm::vec3&, uint32_t face,
                                                        idk::RenderQueue&, idk::glFramebuffer& );

    void                                _convolve_cubemap( uint32_t cubemap,
                                                           uint32_t layer, uint32_t face,
                                                           idk::glShaderProgram&,
                                                           idk::glFramebuffer& );
    // -----------------------------------------------------------------------------------------


    idk::RenderQueue &                  _getRenderQueue( int id ) { return m_render_queues.get(id); };


    // Render stages    
    // ------------------------------------------------------------------------------------
    void RenderStage_envmapping( IDK_Camera&, float dtime );

    void RenderStage_geometry( IDK_Camera &, float dtime,
                               glFramebuffer &buffer_out );



    // void RenderStage_atmospheres( IDK_Camera &,
    //                               glFramebuffer &buffer_in,
    //                               glFramebuffer &buffer_out );


    void RenderStage_radiance();
    void RenderStage_dirlights();
    void RenderStage_pointlights();
    void RenderStage_spotlights();
    void RenderStage_volumetrics( int idx );

    void RenderStage_lighting( IDK_Camera &, float dtime,
                               glFramebuffer &buffer_in,
                               glFramebuffer &buffer_out );


    void PostProcess_bloom( glFramebuffer &buffer_in );

    void PostProcess_chromatic_aberration( glFramebuffer &buffer_in,
                                           glFramebuffer &buffer_out );

    void PostProcess_SSR( glFramebuffer &buffer_in, glFramebuffer &buffer_out );

    void PostProcess_colorgrading( IDK_Camera &,
                                   glFramebuffer &buffer_in,
                                   glFramebuffer &buffer_out );

    void PostProcess_text ( glFramebuffer &buffer_out );
    void PostProcess_ui   ( glFramebuffer &buffer_out );

    void PostProcess_overlay( glFramebuffer &buffer_out );


    void RenderStage_postprocessing( IDK_Camera &,
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
    IDK_Camera &                        getCamera( int cam_id ) { return m_cameras.get(cam_id); };
    IDK_Camera &                        getCamera(            ) { return getCamera(m_active_camera_id);  };
    int                                 activeCamera() { return m_active_camera_id; };
    idk::Allocator<IDK_Camera> &        getCameras() { return m_cameras; };


                                        IDK_ALLOCATOR_ACCESS(Dirlight,   IDK_Dirlight,   m_dirlights);
                                        IDK_ALLOCATOR_ACCESS(Pointlight, IDK_Pointlight, m_pointlights);
                                        IDK_ALLOCATOR_ACCESS(Spotlight,  IDK_Spotlight,  m_spotlights);
                                        // IDK_ALLOCATOR_ACCESS(Atmosphere, IDK_Atmosphere, m_atmospheres);

    int                                 createRenderQueue( const std::string& );
    int                                 createRenderQueue( const std::string&, const idk::RenderQueueConfig& );
    void                                destroyRenderQueue( int RQ );

    int                                 createShadowCasterQueue( const std::string &program );
    void                                destroyShadowCasterQueue( int RQ );


    // idk::LightSystem &                  lightSystem() { return m_lightsystem; };
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
    int                                 loadModelLOD( int model, int level, const std::string &filepath );

    void                                drawSphere( const glm::vec3 position, float radius );
    void                                drawSphere( const glm::mat4& );
    void                                drawSphereWireframe( const glm::mat4& );
    void                                drawRect( const glm::mat4& );
    void                                drawRectWireframe( const glm::mat4& );
    void                                drawLine( const glm::vec3 A, const glm::vec3 B, float thickness );
    void                                drawCapsule( const glm::vec3 A, const glm::vec3 B, float thickness );

    void                                drawModel( int model, const glm::mat4& );
    void                                drawModel( int model, const idk::Transform& );
    void                                drawModelRQ( int RQ, int model, const glm::mat4 & );
    void                                drawShadowCasterRQ( int RQ, int model, const glm::mat4 & );

    void                                drawDecal( int model, const glm::vec3&, const glm::vec3&,
                                                   float );

    void                                drawTextureOverlay( uint32_t texture );

    // void                                drawModelViewspace( int model, const glm::mat4 & );
    void                                drawShadowCaster( int model, const glm::mat4 & );

    /** Draw a model which contributes to global illumination. */
    void                                drawEnvironmental( int model, const glm::mat4 & );


    void                                pushRenderOverlay( const std::string &filepath,
                                                           float fadein, float display,
                                                           float fadeout );

    void                                pushRenderOverlayFill( const glm::vec3 &fill,
                                                               float fadein, float display,
                                                               float fadeout );


    void                                pushRenderOverlay( const RenderOverlay& );
    void                                pushRenderOverlayFill( const RenderOverlayFill& );
    void                                skipRenderOverlay();
    void                                skipAllRenderOverlays();


    int                                 createParticleEmitter( const ParticleEmitter &P );
    void                                destroyParticleEmitter( int emitter );
    ParticleEmitter &                   getParticleEmitter( int emitter );


    int                                 createProgram( const std::string &name,
                                                       const std::string &root,
                                                       const std::string &vs,
                                                       const std::string &fs );


    int                                 createProgram( const std::string &name,
                                                       const idk::glShaderProgram &program );



    idk::glShaderProgram &getProgram( const std::string &name )
    {
        if (m_program_ids.contains(name) == false)
        {
            LOG_ERROR() << "No such shader program: " << name; 
            IDK_ASSERT("No such shader program", false);
        }


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

    void                                applyRenderSettings( const RenderSettings& );
    const RenderSettings&               getRenderSettings();

    glFramebuffer                       m_dirshadow_buffer;
    glFramebuffer                       m_radiance_buffer;
    glFramebuffer                       m_volumetrics_buffers[2];

    glFramebuffer                       m_SSAO_buffers[2];
    uint32_t                            m_SSAO_noise;

    GLuint                              m_velocitybuffer;

    glFramebuffer                       m_mainbuffer_0;
    glFramebuffer                       m_mainbuffer_1;
    glFramebuffer                       m_finalbuffer;

    glFramebuffer                       m_gbuffer;
    glFramebuffer                       m_mip_scratchbuffer;
    glFramebuffer                       m_ui_buffer;

    // auto &getScratchBuffers()  { return m_scratchbuffers; };
    auto &getScratchBuffers2() { return m_scratchbuffers2; };

    idk::glFramebuffer &getUIFrameBuffer() { return m_ui_buffer; };

    inline static SSAO_Settings SSAO_settings;

};




// template <typename ...Args>
// int
// idk::RenderEngine::createProgram( const std::string &name, idk::glShaderStage first, Args... rest )
// {
//     int id = m_programs.create(idk::glShaderProgram(rest...));
//     m_program_ids[name] = id;

//     return id;
// }


