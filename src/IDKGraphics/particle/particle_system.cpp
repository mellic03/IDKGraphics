#include "particle_system.hpp"

#include <libidk/idk_gl.hpp>
#include <libidk/idk_random.hpp>
#include <libidk/idk_wallocator.hpp>
#include <libidk/idk_log.hpp>
#include <libidk/idk_noisegen.hpp>

#include "../idk_renderengine.hpp"
#include "../batching/idk_render_queue.hpp"
#include "../batching/idk_model_allocator.hpp"
#include "../storage/bindings.hpp"


using namespace idk;

#define EMITTER_MAX_PARTICLES 512
#define MAX_EMITTERS 128


using EmitterDesc  = idk::ParticleSystem::EmitterDesc;
using ParticleDesc = idk::ParticleSystem::ParticleDesc;


struct SSBO_Particle
{
    glm::vec4 t   = glm::vec4(0.0f);
    glm::vec4 sc  = glm::vec4(1.0f);
    glm::vec4 pos = glm::vec4(0.0f);
    glm::vec4 vel = glm::vec4(0.0f);
    glm::vec4 col = glm::vec4(1.0f);
};


struct SSBO_ParticleDesc
{
    glm::uvec4 count;
    glm::vec4 pos, posRNG;
    glm::vec4 vel, velRNG;
    glm::vec4 scA, scB, scRNG;
    glm::vec4 dur, durRNG;
    glm::vec4 stime;
};




struct SSBO_ParticleBuffer
{
    SSBO_Particle particles[EMITTER_MAX_PARTICLES * MAX_EMITTERS];
};



namespace
{
    idk::WAllocator<ParticleSystem::Emitter> m_emitters;

    idk::glBufferObject<GL_SHADER_STORAGE_BUFFER> m_SSBO;
    idk::glBufferObject<GL_SHADER_STORAGE_BUFFER> m_SSBO_desc;
    idk::glBufferObject<GL_DRAW_INDIRECT_BUFFER>  m_DIB;
}



void
idk::ParticleSystem::init( idk::RenderEngine &ren )
{
    auto VS = idk::glShaderStage("IDKGE/shaders/particle/particle-gpass.vs");
    auto FS = idk::glShaderStage("IDKGE/shaders/particle/particle-gpass.fs");
    auto CS1 = idk::glShaderStage("IDKGE/shaders/particle/particle-update.comp");
    auto CS2 = idk::glShaderStage("IDKGE/shaders/particle/particle-clear.comp");

    ren.createProgram("particle-gpass", idk::glShaderProgram(VS, FS));
    ren.createProgram("particle-update", idk::glShaderProgram(CS1));
    ren.createProgram("particle-clear", idk::glShaderProgram(CS2));

    m_SSBO.init(shader_bindings::SSBO_Particles);
    m_SSBO.bufferData(sizeof(SSBO_ParticleBuffer), nullptr, GL_DYNAMIC_DRAW);

    m_SSBO_desc.init(shader_bindings::SSBO_ParticleDesc);
    m_SSBO_desc.bufferData(MAX_EMITTERS*sizeof(SSBO_ParticleDesc), nullptr, GL_DYNAMIC_DRAW);

    m_DIB.init();
    m_DIB.bufferData(
        MAX_EMITTERS * EMITTER_MAX_PARTICLES * sizeof(idk::glDrawCmd), nullptr, GL_DYNAMIC_DRAW
    );

    {
        auto &program = ren.getBindProgram("particle-clear");

        for (int id=0; id<MAX_EMITTERS; id++)
        {
            program.set_uint("un_offset", uint32_t(EMITTER_MAX_PARTICLES * id));
            program.dispatch(EMITTER_MAX_PARTICLES/32, 1, 1);
        }
        gl::memoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }
}




void
idk::ParticleSystem::update( idk::RenderEngine &ren, float dt, const glm::vec3 &view )
{
    static std::vector<int> cull(128);
    cull.clear();

    for (auto &[id, emitter]: m_emitters)
    {
        int model = emitter.edesc.model_id;

        if (emitter.edesc.duration > 0.0f && emitter.finished())
        {
            cull.push_back(id);
        }

        else
        {
            emitter.update(dt);
        }
    }

    {
        auto &program = ren.getBindProgram("particle-clear");

        for (int id: cull)
        {
            destroyEmitter(id);
            program.set_uint("un_offset", uint32_t(EMITTER_MAX_PARTICLES * id));
            program.dispatch(EMITTER_MAX_PARTICLES/32, 1, 1);
        }

        gl::memoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }


    static std::vector<SSBO_ParticleDesc> descriptors;
    descriptors.clear();


    auto &program = ren.getBindProgram("particle-update");

    for (auto &[id, emitter]: m_emitters)
    {
        SSBO_ParticleDesc desc = {
            .count    = glm::vec4(emitter.pdesc.count),
            .pos      = glm::vec4(emitter.pdesc.origin, 1.0f),
            .posRNG   = glm::vec4(emitter.pdesc.origin_rng, 1.0f),
            .vel      = glm::vec4(emitter.pdesc.vel, 1.0f),
            .velRNG   = glm::vec4(emitter.pdesc.vel_rng, 1.0f),
            .scA      = glm::vec4(emitter.pdesc.scale_start),
            .scB      = glm::vec4(emitter.pdesc.scale_end),
            .scRNG    = glm::vec4(emitter.pdesc.scale_rng),
            .dur      = glm::vec4(emitter.pdesc.duration),
            .durRNG   = glm::vec4(emitter.pdesc.duration_rng),
            .stime    = glm::vec4(emitter.pdesc.spawn_time)
        };
    
        descriptors.push_back(desc);
        m_SSBO_desc.bufferSubData(
            0, sizeof(SSBO_ParticleDesc) * descriptors.size(), descriptors.data()
        );

        program.set_uint("un_offset", uint32_t(EMITTER_MAX_PARTICLES * id));
        program.dispatch(EMITTER_MAX_PARTICLES/32, 1, 1);
        descriptors.clear();

        gl::memoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }

}




void
idk::ParticleSystem::render( idk::RenderEngine &ren, idk::ModelAllocator &MA )
{
    // glDepthMask(GL_FALSE);
    gl::disable(GL_BLEND);
    glDepthMask(GL_TRUE);
    // gl::enable(GL_BLEND);
    // gl::blendFunc(GL_SRC_ALPHA, GL_ONE);

    static const idk::glTextureConfig config = {
        .target         = GL_TEXTURE_2D,
        .internalformat = GL_RGBA8,
        .format         = GL_RGBA,
        .minfilter      = GL_NEAREST,
        .magfilter      = GL_LINEAR,
        .wrap_s         = GL_REPEAT,
        .wrap_t         = GL_REPEAT,
        .datatype       = GL_UNSIGNED_BYTE,
        .genmipmap      = GL_TRUE,
        .bindless       = GL_FALSE
    };

    static uint32_t texture = gltools::loadTexture("assets/textures/fire.png", config);

    static std::vector<idk::glDrawCmd> commands;
    commands.clear();


    auto &program = ren.getBindProgram("particle-gpass");

    for (auto &[id, emitter]: m_emitters)
    {
        int model_id = emitter.edesc.model_id;

        auto &mesh = MA.getModel(model_id).meshes[0];
        {
            idk::glDrawCmd cmd = {
                .count         = mesh.numIndices,
                .instanceCount = uint32_t(emitter.pdesc.count),
                .firstIndex    = mesh.firstIndex,
                .baseVertex    = mesh.baseVertex,
                .baseInstance  = 0
            };

            commands.push_back(cmd);
        }

        auto &mat = MA.getMaterial(mesh.material);
        program.set_sampler2D("un_albedo", 0, mat.textures[0]);
        program.set_uint("un_offset", uint32_t(EMITTER_MAX_PARTICLES * id));

        m_DIB.bufferSubData(0, sizeof(idk::glDrawCmd) * commands.size(), commands.data());
        gl::bindBuffer(GL_DRAW_INDIRECT_BUFFER, m_DIB.ID());

        gl::multiDrawElementsIndirect(
            GL_TRIANGLES,
            GL_UNSIGNED_INT,
            nullptr,
            commands.size(),
            sizeof(idk::glDrawCmd)
        );

        commands.clear();
    }

    gl::disable(GL_BLEND);
    glDepthMask(GL_TRUE);

}





ParticleSystem::Emitter::Emitter( const ParticleDesc &pd, const EmitterDesc &ed )
:   m_timer(0.0f), pdesc(pd), edesc(ed)
{

}


void
ParticleSystem::Emitter::update( float dt )
{
    if (edesc.duration > 0.0f)
    {
        m_timer += dt;
    }
}




int
idk::ParticleSystem::createEmitter( const EmitterDesc &ed, const ParticleDesc &pd )
{
    int id = m_emitters.create(Emitter(pd, ed));
    LOG_DEBUG() << "Created particle emitter with id " << id;
    return id;
}



void
idk::ParticleSystem::destroyEmitter( int id )
{
    LOG_DEBUG() << "Destroyed particle emitter with id " << id;
    m_emitters.destroy(id);
}


idk::ParticleSystem::Emitter&
idk::ParticleSystem::getEmitter( int id )
{
    return m_emitters.get(id);
}





