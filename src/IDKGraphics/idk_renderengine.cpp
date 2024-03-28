#include "idk_renderengine.hpp"
#include "render/idk_initflags.hpp"
#include "render/idk_vxgi.hpp"

#include <libidk/GL/idk_glShaderStage.hpp>
#include <libidk/idk_image.hpp>
#include <libidk/idk_texturegen.hpp>


static float delta_time = 1.0f;


void
idk::RenderEngine::init_screenquad()
{
    float quad_vertices[] = {
      -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,
      -1.0f, -1.0f,  0.0f,  0.0f,  0.0f,
       1.0f, -1.0f,  0.0f,  1.0f,  0.0f,

      -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,
       1.0f, -1.0f,  0.0f,  1.0f,  0.0f,
       1.0f,  1.0f,  0.0f,  1.0f,  1.0f
    };

    // Send screen quad to GPU
    // ------------------------------------------------------------------------------------
    gl::genVertexArrays(1, &m_quad_VAO);
    gl::genBuffers(1, &m_quad_VBO);

    gl::bindVertexArray(m_quad_VAO);
    gl::bindBuffer(GL_ARRAY_BUFFER, m_quad_VBO);
    gl::bufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

    gl::vertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), 0);
    gl::enableVertexAttribArray(0);

    gl::vertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), 3*sizeof(float));
    gl::enableVertexAttribArray(1);
    // ------------------------------------------------------------------------------------
}

void
idk::RenderEngine::compileShaders()
{
    // createProgram("vxgi-voxelize", "IDKGE/shaders/vxgi/", "voxelize.vs", "voxelize.fs");
    // createProgram("vxgi-shadow",   "IDKGE/shaders/vxgi/", "shadow.vs", "shadow.fs");
    // createProgram("vxgi-trace",    "IDKGE/shaders/deferred/", "background.vs", "../vxgi/trace.fs");
    // createProgram("vxgi-inject",    glShaderProgram("IDKGE/shaders/vxgi/inject-radiance.comp"));
    // createProgram("vxgi-propagate", glShaderProgram("IDKGE/shaders/vxgi/propagate-radiance.comp"));

    // createProgram("vxgi-mipmap-1",  glShaderProgram("IDKGE/shaders/vxgi/mipmap-1.comp"));
    // createProgram("vxgi-mipmap-2",  glShaderProgram("IDKGE/shaders/vxgi/mipmap-2.comp"));
    // createProgram("vxgi-clear",     glShaderProgram("IDKGE/shaders/vxgi/clear.comp"));
    // createProgram("vxgi-copy",      glShaderProgram("IDKGE/shaders/vxgi/copy.comp"));


    createProgram("gpass", "IDKGE/shaders/deferred/", "gpass.vs", "gpass.fs");
    createProgram("gpass-viewspace", "IDKGE/shaders/deferred/", "gpass-viewspace.vs", "gpass.fs");
    createProgram("lpass", "IDKGE/shaders/", "screenquad.vs", "deferred/light-pass.fs");


    createProgram("planet-gen", glShaderProgram("IDKGE/shaders/generative/planet-gen.comp"));


    idk::glShaderStage VS_Pointlight("IDKGE/shaders/deferred/pointlight.vs");
    idk::glShaderStage FS_Pointlight("IDKGE/shaders/deferred/pointlight.fs");
    createProgram("deferred-pointlight", idk::glShaderProgram(VS_Pointlight, FS_Pointlight));

    idk::glShaderStage VS_Spotlight("IDKGE/shaders/deferred/spotlight.vs");
    idk::glShaderStage FS_Spotlight("IDKGE/shaders/deferred/spotlight.fs");
    createProgram("deferred-spotlight", idk::glShaderProgram(VS_Spotlight, FS_Spotlight));

    idk::glShaderStage VS_Atmosphere("IDKGE/shaders/deferred/atmosphere.vs");
    idk::glShaderStage FS_Atmosphere("IDKGE/shaders/deferred/atmosphere.fs");
    createProgram("atmosphere", idk::glShaderProgram(VS_Atmosphere, FS_Atmosphere));

    createProgram("dirshadow-indirect", "IDKGE/shaders/", "dirshadow-indirect.vs", "dirshadow.fs");
    createProgram("dir-volumetric", "IDKGE/shaders/", "screenquad.vs", "deferred/volumetric_dirlight.fs");

    createProgram("SSR", glShaderProgram("IDKGE/shaders/post/SSR.comp"));
    // createProgram("motion-blur", glShaderProgram("IDKGE/shaders/post/motion-blur.comp"));
    createProgram("bloom-first", "IDKGE/shaders/", "screenquad.vs", "post/bloom-first.fs");
    createProgram("bloom-down",  "IDKGE/shaders/", "screenquad.vs", "post/bloom-down.fs");
    createProgram("bloom-up",    "IDKGE/shaders/", "screenquad.vs", "post/bloom-up.fs");


    createProgram("additive",       "IDKGE/shaders/", "screenquad.vs", "post/additive.fs");
    createProgram("blit",           "IDKGE/shaders/", "screenquad.vs", "post/blit.fs");
    createProgram("fxaa",           "IDKGE/shaders/", "screenquad.vs", "post/fxaa.fs");
    createProgram("colorgrade",     "IDKGE/shaders/", "screenquad.vs", "post/colorgrade.fs");
    createProgram("dir_shadow",     "IDKGE/shaders/", "dirshadow.vs",  "dirshadow.fs");

}



void
idk::RenderEngine::init_framebuffers( int w, int h )
{
    w = glm::clamp(w, 128, 4096);
    h = glm::clamp(h, 128, 4096);


    idk::glTextureConfig config = {
        .internalformat = GL_RGBA16F,
        .minfilter      = GL_LINEAR,
        .magfilter      = GL_LINEAR,
        .datatype       = GL_FLOAT,
        .genmipmap      = GL_FALSE
    };

    static const idk::DepthAttachmentConfig depth_config = {
        .internalformat = GL_DEPTH_COMPONENT24,
        .datatype       = GL_FLOAT
    };


    // m_vxgi_buffer.reset(VXGI_TEXTURE_SIZE, VXGI_TEXTURE_SIZE, 1);
    // m_vxgi_buffer.colorAttachment(0, config);
    // m_vxgi_buffer.depthAttachment(depth_config);

    m_finalbuffer.reset(w, h, 1);
    m_finalbuffer.colorAttachment(0, config);

    m_mainbuffer_0.reset(w, h, 2);
    m_mainbuffer_0.colorAttachment(0, config);
    m_mainbuffer_0.colorAttachment(1, config);

    m_mainbuffer_1.reset(w, h, 2);
    m_mainbuffer_1.colorAttachment(0, config);
    m_mainbuffer_1.colorAttachment(1, config);


    idk::glTextureConfig albedo_config = {
        .internalformat = GL_RGBA16F,
        .format         = GL_RGBA,
        .minfilter      = GL_LINEAR,
        .magfilter      = GL_LINEAR,
        .datatype       = GL_FLOAT,
        .genmipmap      = GL_FALSE
    };

    idk::glTextureConfig normal_config = {
        .internalformat = GL_RGB16F,
        .format         = GL_RGB,
        .minfilter      = GL_LINEAR,
        .magfilter      = GL_LINEAR,
        .datatype       = GL_FLOAT,
        .genmipmap      = GL_FALSE
    };

    idk::glTextureConfig pbr_config = {
        .internalformat = GL_RGBA16F,
        .format         = GL_RGBA,
        .minfilter      = GL_LINEAR,
        .magfilter      = GL_LINEAR,
        .datatype       = GL_FLOAT,
        .genmipmap      = GL_FALSE
    };


    m_geom_buffer.reset(w, h, 3);
    m_geom_buffer.colorAttachment(0, albedo_config);
    m_geom_buffer.colorAttachment(1, normal_config);
    m_geom_buffer.colorAttachment(2, pbr_config);
    m_geom_buffer.depthAttachment(depth_config);


    for (int i=0; i<4; i++)
    {
        int width  = w / pow(2, i);
        int height = h / pow(2, i);

        m_scratchbuffers[i].reset(width, height, 1);
        m_scratchbuffers[i].colorAttachment(0, config);
    }


    config = {
        .internalformat = GL_RGBA16F,
        .format         = GL_RGBA,
        .minfilter      = GL_LINEAR,
        .magfilter      = GL_LINEAR,
        .datatype       = GL_FLOAT,
        .genmipmap      = GL_FALSE
    };

    for (int i=0; i<=BLOOM_MAX_LEVEL; i++)
    {
        m_bloom_buffers[i].reset(w/pow(2, i), h/pow(2, i), 1);
        m_bloom_buffers[i].colorAttachment(0, config);
    }

}


void
idk::RenderEngine::init_all( std::string name, int w, int h )
{
    compileShaders();

    init_screenquad();
    init_framebuffers(w, h);

    m_lightsystem.init();


    m_RQ           = m_render_queues.create();
    m_viewspace_RQ = m_render_queues.create();
    m_shadow_RQ    = m_render_queues.create();
    m_GI_RQ        = m_render_queues.create();



    // Initialize GL_DRAW_INDIRECT_BUFFER for glMultiDrawElementsIndirect
    // -----------------------------------------------------------------------------------------
    m_DrawCommandBuffer.init();
    m_DrawCommandBuffer.bufferData(512 * sizeof(idk::glDrawCmd), nullptr, GL_DYNAMIC_COPY);

    m_DrawIndirectData = new idk::DrawIndirectData;
    m_DrawIndirectSSBO.init(0);
    // -----------------------------------------------------------------------------------------


    // Deferred light sources
    // -----------------------------------------------------------------------------------------
    m_unit_sphere = modelAllocator().loadModel("IDKGE/resources/unit-sphere.idkvi");
    m_unit_cone   = modelAllocator().loadModel("IDKGE/resources/unit-cone.idkvi");
    m_unit_line   = modelAllocator().loadModel("IDKGE/resources/unit-line.idkvi");
    // -----------------------------------------------------------------------------------------

    m_UBO_RenderData.init(3);

    m_UBO_dirlights.init();
    m_UBO_dirlights.bind(5);
    m_UBO_dirlights.bufferData(IDK_MAX_DIRLIGHTS * (sizeof(Dirlight) + sizeof(glm::mat4)), nullptr);


    // Allocate space for atmosphere cubemap
    // -----------------------------------------------------------------------------------------
    static const glTextureConfig atmosphere_config = {
        .internalformat = GL_RGBA16F,
        .format         = GL_RGBA,
        .minfilter      = GL_LINEAR_MIPMAP_LINEAR,
        .magfilter      = GL_LINEAR,
        .datatype       = GL_FLOAT,
        .genmipmap      = GL_TRUE
    };
    m_skybox = gltools::loadCubemap2(256, nullptr, atmosphere_config);
    gl::clearTexImage(m_skybox, 0, GL_RGBA, GL_FLOAT, nullptr);
    // -----------------------------------------------------------------------------------------



    // Compute BRDF LUT
    // -----------------------------------------------------------------------------------------
    idk::glTextureConfig config = {
        .internalformat = GL_RG16F,
        .format         = GL_RG,
        .minfilter      = GL_LINEAR,
        .magfilter      = GL_LINEAR,
        .wrap_s         = GL_CLAMP_TO_EDGE,
        .wrap_t         = GL_CLAMP_TO_EDGE,
        .datatype       = GL_FLOAT,
        .genmipmap      = GL_FALSE
    };

    static constexpr size_t LUT_TEXTURE_SIZE = 512;

    BRDF_LUT = idk::gltools::loadTexture2D(LUT_TEXTURE_SIZE, LUT_TEXTURE_SIZE, nullptr, config);
    idk::gl::bindImageTexture(0, BRDF_LUT, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG16F);

    idk::glShaderProgram program("IDKGE/shaders/brdf-lut.comp");
    program.bind();
    program.dispatch(LUT_TEXTURE_SIZE/8, LUT_TEXTURE_SIZE/8, 1);
    idk::gl::memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    program.unbind();
    // -----------------------------------------------------------------------------------------


    m_active_camera_id = createCamera();

    loadSkybox("IDKGE/resources/skybox/");

    // vxgi_albedo = VXGI::allocateAlbedoTexture(VXGI_TEXTURE_SIZE);
    // vxgi_normal = VXGI::allocateNormalTexture(VXGI_TEXTURE_SIZE);

    // for (int i=0; i<6; i++)
    // {
    //     vxgi_radiance[i] = VXGI::allocateTexture(VXGI_TEXTURE_SIZE);
    //     vxgi_radiance_2[i] = VXGI::allocateTexture(VXGI_TEXTURE_SIZE);
    // }

}



idk::RenderEngine::RenderEngine( const std::string &name, int w, int h, int gl_major,
                                 int gl_minor, uint32_t flags )
:
    m_windowsys(name.c_str(), w, h, gl_major, gl_minor, flags)
{
    m_resolution = glm::ivec2(w, h);
    gl::enable(GL_DEPTH_TEST, GL_CULL_FACE);

    if (flags == idk::InitFlag::NONE)
    {
        init_all(name, w, h);
        return;
    }

    if (flags & idk::InitFlag::INIT_HEADLESS)
    {
        return;
    }
}


int
idk::RenderEngine::createProgram( const std::string &name, const std::string &root,
                                  const std::string &vs, const std::string &fs )
{
    int id = m_programs.create();
    m_programs.get(id).loadFileC(root, vs, fs);
    m_program_ids[name] = id;

    return id;
}


int
idk::RenderEngine::createProgram( const std::string &name, const idk::glShaderProgram &program )
{
    int id = m_programs.create(program);
    m_program_ids[name] = id;
    return id;
}



int
idk::RenderEngine::createRenderQueue( const std::string &program )
{
    int ID = m_user_render_queues.create();
    m_user_render_queues.get(ID).name = program;
    return ID;
}


void
idk::RenderEngine::destroyRenderQueue( int RQ )
{
    m_user_render_queues.destroy(RQ);
}




int
idk::RenderEngine::loadSkybox( const std::string &filepath )
{
    gl::enable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    static const std::vector<std::string> faces
    {
        "px.png", "nx.png", "py.png", "ny.png", "pz.png", "nz.png"
    };

    static const std::vector<std::string> faces2
    {
        "0px.png", "0nx.png", "0py.png", "0ny.png", "0pz.png", "0nz.png"
    };

    static const glTextureConfig skybox_config = {
        .internalformat = GL_SRGB8_ALPHA8,
        .format         = GL_RGBA,
        .minfilter      = GL_LINEAR,
        .magfilter      = GL_LINEAR,
        .datatype       = GL_UNSIGNED_BYTE,
        .genmipmap      = false
    };

    static const glTextureConfig diffuse_config = {
        .internalformat = GL_SRGB8_ALPHA8,
        .format         = GL_RGBA,
        .minfilter      = GL_LINEAR,
        .magfilter      = GL_LINEAR,
        .datatype       = GL_UNSIGNED_BYTE,
        .genmipmap      = true
    };


    static const glTextureConfig specular_config = {
        .internalformat = GL_SRGB8_ALPHA8,
        .format         = GL_RGBA,
        .minfilter      = GL_LINEAR_MIPMAP_LINEAR,
        .magfilter      = GL_LINEAR,
        .datatype       = GL_UNSIGNED_BYTE,
        .genmipmap      = false,
        .setmipmap      = true,
        .texbaselevel   = 0,
        .texmaxlevel    = 5
    };

    GLuint skybox   = gltools::loadCubemap(filepath, faces, skybox_config);
    GLuint diffuse  = gltools::loadCubemap(filepath + "diffuse/", faces, diffuse_config);
    GLuint specular = gltools::loadCubemap(filepath + "specular/", faces2, specular_config);

    for (GLint mip=0; mip<=5; mip++)
    {
        gltools::loadCubemapMip(filepath + "specular/", faces, specular_config, specular, mip);
    }

    skyboxes.push_back(skybox);
    skyboxes_IBL.push_back(std::make_pair(diffuse, specular));

    return skyboxes.size() - 1;
}



void
idk::RenderEngine::f_fbfb( glShaderProgram &program, glFramebuffer &in )
{
    gl::bindFramebuffer(GL_FRAMEBUFFER, 0);

    for (size_t i=0; i < in.attachments.size(); i++)
        program.set_sampler2D("un_texture_" + std::to_string(i), in.attachments[i]);

    gl::drawArrays(GL_TRIANGLES, 0, 6);
}



void
idk::RenderEngine::tex2tex( glShaderProgram &program, glFramebuffer &in, glFramebuffer &out )
{
    out.bind();

    for (size_t i=0; i < in.attachments.size(); i++)
    {
        program.set_sampler2D("un_texture_" + std::to_string(i), in.attachments[i]);
    }

    gl::drawArrays(GL_TRIANGLES, 0, 6);
};



void
idk::RenderEngine::tex2tex( glShaderProgram &program, glFramebuffer &a, glFramebuffer &b, glFramebuffer &out )
{
    out.bind();

    size_t textureID = 0;
    for (size_t i=0; i < a.attachments.size(); i++)
    {
        program.set_sampler2D("un_texture_" + std::to_string(i), a.attachments[i]);
    }

    for (size_t i=0; i < b.attachments.size(); i++)
    {
        program.set_sampler2D("un_texture_" + std::to_string(4+i), b.attachments[i]);
    }

    gl::drawArrays(GL_TRIANGLES, 0, 6);
};


void
idk::RenderEngine::setRenderSetting( idk::RenderSetting flag, bool value )
{
    if (value)
    {
        m_render_settings |= static_cast<uint32_t>(flag);
    }

    else
    {
        m_render_settings &= ~static_cast<uint32_t>(flag);
    }
}


bool
idk::RenderEngine::getRenderSetting( idk::RenderSetting flag )
{
    return m_render_settings & static_cast<uint32_t>(flag);
}




int
idk::RenderEngine::createCamera()
{
    int camera_id = m_camera_allocator.create();
    m_camera_allocator.get(camera_id).aspect = float(m_resolution.x) / float(m_resolution.y);
    return camera_id;
}


void
idk::RenderEngine::drawLine( const glm::vec3 A, const glm::vec3 B, float thickness )
{
    glm::mat4 TR = glm::inverse(glm::lookAt(A, B, glm::vec3(0.0f, 1.0f, 0.0f)));
    glm::mat4 S  = glm::scale(glm::mat4(1.0f), glm::vec3(thickness, thickness, glm::length(B - A)));

    drawModel(m_unit_line, TR*S);
}


int
idk::RenderEngine::loadModel( const std::string &filepath )
{
    return m_model_allocator.loadModel(filepath);
}


void
idk::RenderEngine::drawModel( int model, const glm::mat4 &transform )
{
    _getRenderQueue(m_RQ).enque(model, transform);
}


void
idk::RenderEngine::drawModelRQ( int RQ, int model, const glm::mat4 &transform )
{
    m_user_render_queues.get(RQ).enque(model, transform);
}


void
idk::RenderEngine::drawModelViewspace( int model, const glm::mat4 &transform )
{
    _getRenderQueue(m_viewspace_RQ).enque(model, transform);
}


void
idk::RenderEngine::drawShadowCaster( int model, const glm::mat4 &transform )
{
    _getRenderQueue(m_shadow_RQ).enque(model, transform);
}


void
idk::RenderEngine::drawEnvironmental( int model, const glm::mat4 &transform )
{
    _getRenderQueue(m_GI_RQ).enque(model, transform);
}



void
idk::RenderEngine::update_UBO_camera()
{
    idk::Camera &camera = getCamera();

    static glm::mat4 prev_v, prev_p, prev_pv;
    static glm::vec3 prev_position;

    glm::mat4 P = camera.P();
    glm::mat4 V = camera.V();

    m_RenderData.cameras[0].position = glm::vec4(camera.position, 1.0f);
    m_RenderData.cameras[0].V  = V;
    m_RenderData.cameras[0].P  = P;
    m_RenderData.cameras[0].PV = P * V;

    glm::mat4 P_far = glm::perspective(glm::radians(camera.fov), camera.aspect, camera.near, 10000.0f);
    m_RenderData.cameras[0].P_far = P_far;
    m_RenderData.cameras[0].PV_far = P_far * V;

    m_RenderData.cameras[0].image_size  = glm::vec4(width(), height(), 0.0f, 0.0f);
    m_RenderData.cameras[0].image_plane = glm::vec4(camera.near, camera.far, 0.0f, 0.0f);
    m_RenderData.cameras[0].bloom       = camera.bloom;

    m_RenderData.cameras[0].prev_position = glm::vec4(prev_position, 1.0f);
    m_RenderData.cameras[0].prev_V  = prev_v;
    m_RenderData.cameras[0].prev_P  = prev_p;
    m_RenderData.cameras[0].prev_PV = prev_pv;

    prev_position = camera.position;
    prev_v  = V;
    prev_p  = P;
    prev_pv = prev_p * prev_v;
    
}


idk::glDrawCmd
idk::RenderEngine::genLightsourceDrawCommand( int model, uint32_t num_lights, idk::ModelAllocator &MA )
{
    int mesh_id = MA.getModel(model).mesh_ids[0];
    MeshDescriptor &mesh = MA.getMesh(mesh_id);

    idk::glDrawCmd cmd = {
        .count         = mesh.numIndices,
        .instanceCount = num_lights,
        .firstIndex    = mesh.firstIndex,
        .baseVertex    = mesh.baseVertex,
        .baseInstance  = 0
    };

    return cmd;
}


idk::glDrawCmd
idk::RenderEngine::genAtmosphereDrawCommand( idk::ModelAllocator &MA )
{
    int mesh_id = MA.getModel(m_unit_sphere).mesh_ids[0];
    MeshDescriptor &mesh = MA.getMesh(mesh_id);

    idk::glDrawCmd cmd = {
        .count         = mesh.numIndices,
        .instanceCount = uint32_t(m_atmospheres.size()),
        .firstIndex    = mesh.firstIndex,
        .baseVertex    = mesh.baseVertex,
        .baseInstance  = 0
    };

    return cmd;
}


void
idk::RenderEngine::updateLightsourcesUBO( idk::UBORenderData &data )
{
    static uint32_t offset;

    // offset = 0;
    // for (IDK_Dirlight &light: m_dirlights)
    // {
    //     data.dirlights[offset++] = light;
    // }

    offset = 0;
    for (IDK_Pointlight &light: m_pointlights)
    {
        glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(light.position));
        glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f*light.radius));

        light.transform = T * S;

        data.pointlights[offset++] = light;
    }

    offset = 0;
    for (IDK_Spotlight &light: m_spotlights)
    {
        float h      = light.radius;
        float alpha  = light.angle[0];
        float radius = h * tan(alpha);

        glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(light.position));
        glm::mat4 R = glm::mat4_cast(light.orientation);
        glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f*radius));

        light.transform = T * R * S;
        light.direction = light.orientation * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);

        data.spotlights[offset++] = light;
    }
}


void
idk::RenderEngine::updateAtmosphereUBO( idk::UBORenderData &data )
{
    static uint32_t offset;

    offset = 0;
    for (IDK_Atmosphere &atmosphere: m_atmospheres)
    {
        glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(atmosphere.position));
        glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f * atmosphere.radius * atmosphere.atmosphere_scale));

        atmosphere.transform = T * S;

        // data.atmospheres[offset++] = atmosphere;
    }

    std::memcpy(data.atmospheres, m_atmospheres.data(), m_atmospheres.size() * sizeof(IDK_Atmosphere));

}


void
idk::RenderEngine::update_UBO_dirlights( idk::UBORenderData &data )
{
    idk::Camera &cam = getCamera();

    std::vector<Dirlight>  lights(IDK_MAX_DIRLIGHTS);
    std::vector<glm::mat4> matrices(IDK_MAX_DIRLIGHTS);

    std::vector<idk::Dirlight> &dirlights = m_lightsystem.dirlights();
    idk::Dirlight &light = dirlights[0];
    const glm::vec3 dir = glm::normalize(glm::vec3(light.direction));

    const auto &cascade_matrices = m_lightsystem.depthCascade().getCascadeMatrices();

    for (int j=0; j<cascade_matrices.size(); j++)
    {
        lights[0] = light;
    }

    m_UBO_dirlights.bind();
    m_UBO_dirlights.add(IDK_MAX_DIRLIGHTS * sizeof(Dirlight),  lights.data());
    m_UBO_dirlights.add(glDepthCascade::NUM_CASCADES * sizeof(glm::mat4), cascade_matrices.data());
    m_UBO_dirlights.unbind();
}


void
idk::RenderEngine::beginFrame()
{
    gl::bindFramebuffer(GL_FRAMEBUFFER, 0);
    gl::viewport(0, 0, m_resolution.x, m_resolution.y);
    gl::clearColor(0.0f, 0.0f, 0.0f, 1.0f);
    gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


}


void
idk::RenderEngine::endFrame( float dt )
{
    delta_time = dt;

    m_mainbuffer_0.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_mainbuffer_1.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_finalbuffer.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_DrawCommandBuffer.bind();
    gl::bindVertexArray(m_model_allocator.getVAO());

    // Update SSBO data
    // -----------------------------------------------------------------------------------------
    // m_DrawIndirectSSBO.update(*m_DrawIndirectData);
    // -----------------------------------------------------------------------------------------

    gl::disable(GL_CULL_FACE);
    this->shadowpass();
    gl::enable(GL_CULL_FACE);


    // Update UBO data
    // -----------------------------------------------------------------------------------------
    update_UBO_camera();
    update_UBO_dirlights(m_RenderData);

    updateLightsourcesUBO(m_RenderData);
    updateAtmosphereUBO(m_RenderData);
    m_UBO_RenderData.update(m_RenderData);
    // -----------------------------------------------------------------------------------------



    idk::Camera &camera = getCamera();

    RenderStage_geometry(camera, dt, m_geom_buffer);
    RenderStage_lighting(camera, dt, m_geom_buffer,  m_mainbuffer_0);


    // auto &program = getProgram("SSR");
    // program.bind();

    // gl::bindImageTexture(0, m_mainbuffer_0.attachments[0], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
    // gl::bindImageTexture(1, m_mainbuffer_1.attachments[0], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);

    // program.set_sampler2D("un_albedo", m_geom_buffer.attachments[0]);
    // program.set_sampler2D("un_normal", m_geom_buffer.attachments[1]);
    // program.set_sampler2D("un_pbr",    m_geom_buffer.attachments[2]);
    // program.set_sampler2D("un_fragdepth", m_geom_buffer.depth_attachment);
    // // program.set_sampler2D("un_BRDF_LUT",  BRDF_LUT);

    // program.dispatch(width()/8, height()/8, 1);
    // gl::memoryBarrier(GL_ALL_BARRIER_BITS);


    RenderStage_postprocessing(camera, m_mainbuffer_0, m_finalbuffer);


    gl::enable(GL_DEPTH_TEST, GL_CULL_FACE);


    // Clear render queues
    // -----------------------------------------------------------------------------------------
    for (idk::RenderQueue &queue: m_render_queues)
    {
        queue.clear();
    }

    for (idk::RenderQueue &queue: m_user_render_queues)
    {
        queue.clear();
    }
    // -----------------------------------------------------------------------------------------
}


void
idk::RenderEngine::swapWindow()
{
    SDL_GL_SwapWindow(this->getWindow());
}


void
idk::RenderEngine::resize( int w, int h )
{
    m_resolution.x = w;  m_resolution.y = h;
    init_framebuffers(w, h);
    getCamera().aspect = float(w) / float(h);
}










static void
idk_temp_VXGI_code()
{
    // static bool vxgi_on = getRenderSetting(RenderSetting::VXGI);

    // if (vxgi_on != getRenderSetting(RenderSetting::VXGI))
    // {
    //     auto &program = getProgram("lpass");
        
    //     if (vxgi_on)    program.setDefinition("VXGI_ON", "0");
    //     else            program.setDefinition("VXGI_ON", "1");

    //     program.compile();
    // }



    // vxgi_on = getRenderSetting(RenderSetting::VXGI);

    // if (getRenderSetting(RenderSetting::VXGI))
    // {
    //     gl::bindImageTextures(0, 6, &vxgi_radiance_2[0]);
    //     gl::bindImageTextures(8, 6, &vxgi_radiance[0]);
    //     gl::bindImageTexture(6, vxgi_albedo, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8UI);
    //     gl::bindImageTexture(7, vxgi_normal, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8UI);


    //     if (getRenderSetting(RenderSetting::VXGI_LIVE_VOXELIZATION))
    //     {
    //         // Clear volume textures
    //         // -----------------------------------------------------------------------------------------
    //         {
    //             auto &program = getProgram("vxgi-clear");
    //             program.bind();
    //             program.dispatch(VXGI_TEXTURE_SIZE / 4);
    //             gl::memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    //         }
    //         // -----------------------------------------------------------------------------------------


    //         VXGI::shadowPass(
    //             m_vxgi_buffer,
    //             camera,
    //             lightSystem().getDirlight(0).direction,
    //             getProgram("vxgi-shadow"),
    //             modelAllocator().getDrawIndirectBuffer(),
    //             commands
    //         );


    //         VXGI::renderTexture(
    //             m_vxgi_buffer,
    //             camera,
    //             getProgram("vxgi-voxelize"),
    //             modelAllocator().getDrawIndirectBuffer(),
    //             commands,
    //             m_lightsystem.depthCascade()
    //         );

    //     }

    //     static int vxgi_face   = 0;
    //     static int vxgi_offset = 0;

    //     vxgi_face = (vxgi_face + 1) % 6;
    //     vxgi_offset = (vxgi_offset + 1) % 4;

    //     if (getRenderSetting(RenderSetting::VXGI_LIVE_INJECTION))
    //     {
    //         {
    //             auto &program = getProgram("vxgi-inject");
    //             program.bind();
    //             program.set_int("un_face", vxgi_face);
    //             program.set_int("un_offset", vxgi_offset);

    //             VXGI::injectRadiance(
    //                 program,
    //                 camera,
    //                 m_vxgi_buffer,
    //                 lightSystem().getDirlight(0).direction,
    //                 lightSystem().depthCascade()
    //             );
    //         }
    //         // VXGI::generateMipmap(getProgram("vxgi-mipmap-1"), getProgram("vxgi-mipmap-2"), vxgi_radiance_2);

    //         // {
    //         //     auto &program = getProgram("vxgi-propagate");
    //         //     program.bind();
    //         //     program.set_int("un_face", vxgi_face);
    //         //     program.set_int("un_offset", vxgi_offset);

    //         //     for (int i=0; i<6; i++)
    //         //     {
    //         //         program.set_sampler2D("un_input_radiance[" + std::to_string(i) + "]", vxgi_radiance_2[i]);
    //         //     }
    //         //     program.dispatch(VXGI_TEXTURE_SIZE/4);
    //         //     gl::memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    //         // }
    //         VXGI::generateMipmap(getProgram("vxgi-mipmap-1"), getProgram("vxgi-mipmap-2"), vxgi_radiance);
    //     }
    // }

}




