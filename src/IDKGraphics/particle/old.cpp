// #include "particle_system.hpp"

// #include <libidk/idk_gl.hpp>
// #include <libidk/idk_random.hpp>
// #include <libidk/idk_wallocator.hpp>
// #include <libidk/idk_log.hpp>
// #include <libidk/idk_noisegen.hpp>

// #include "../batching/idk_render_queue.hpp"
// #include "../batching/idk_model_allocator.hpp"
// #include "../storage/bindings.hpp"


// using namespace idk;

// #define EMITTER_MAX_PARTICLES 64
// #define MAX_EMITTERS 128

// struct SSBO_ParticleDesc
// {
//     glm::vec4 count;

//     glm::vec4 origin;
//     glm::vec4 origin_rng;

//     glm::vec4 velocity;
//     glm::vec4 velocity_rng;

//     glm::vec4 scale;
//     glm::vec4 scale_factor;
//     glm::vec4 scale_rng;

//     glm::vec4 timer;
//     glm::vec4 duration;
//     glm::vec4 duration_rng;
// };


// struct SSBO_Particle
// {
//     glm::vec4 timer = glm::vec4(0.0f);
//     glm::vec4 scale = glm::vec4(1.0f);

//     glm::vec4 pos   = glm::vec4(0.0f);
//     glm::vec4 vel   = glm::vec4(0.0f);
//     glm::vec4 rot   = glm::vec4(0.0f);
//     glm::vec4 color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
// };


// struct SSBO_Emitter
// {
//     SSBO_Particle particles[EMITTER_MAX_PARTICLES * MAX_EMITTERS];
// };


// namespace
// {
//     idk::WAllocator<ParticleSystem::Emitter> m_emitters;

//     uint32_t                m_quad_VAO;
//     uint32_t                m_quad_VBO;

//     uint32_t                m_noise;

//     idk::glShaderProgram    m_program_gpass;
//     idk::glShaderProgram    m_program_update;
//     idk::glShaderProgram    m_program_clear;

//     idk::glBufferObject<GL_SHADER_STORAGE_BUFFER> m_SSBO;
//     idk::glBufferObject<GL_SHADER_STORAGE_BUFFER> m_SSBO_desc;
//     idk::glBufferObject<GL_DRAW_INDIRECT_BUFFER>  m_DIB;
// }


// static void ParticleSystem_genQuadVAO();


// void
// idk::ParticleSystem::init()
// {
//     auto VS = idk::glShaderStage("IDKGE/shaders/particle/particle-gpass.vs");
//     auto FS = idk::glShaderStage("IDKGE/shaders/particle/particle-gpass.fs");
//     auto CS1 = idk::glShaderStage("IDKGE/shaders/particle/particle-update.comp");
//     auto CS2 = idk::glShaderStage("IDKGE/shaders/particle/particle-clear.comp");

//     m_program_gpass  = idk::glShaderProgram(VS, FS);
//     m_program_update = idk::glShaderProgram(CS1);
//     m_program_clear  = idk::glShaderProgram(CS2);

//     ParticleSystem_genQuadVAO();

//     m_SSBO.init(shader_bindings::SSBO_Particles);
//     m_SSBO.bufferData(sizeof(SSBO_Emitter), nullptr, GL_DYNAMIC_DRAW);

//     m_SSBO_desc.init(shader_bindings::SSBO_ParticleDesc);
//     m_SSBO_desc.bufferData(MAX_EMITTERS * sizeof(SSBO_ParticleDesc),  nullptr, GL_DYNAMIC_DRAW);

//     m_DIB.init();
//     m_DIB.bufferData(
//         MAX_EMITTERS * EMITTER_MAX_PARTICLES * sizeof(idk::glDrawCmd), nullptr, GL_DYNAMIC_DRAW
//     );

//     {
//         auto &program = m_program_clear;
//         program.bind();

//         for (int id=0; id<MAX_EMITTERS; id++)
//         {
//             program.set_uint("un_offset", uint32_t(EMITTER_MAX_PARTICLES * id));

//             gl::memoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
//             program.dispatch(1, 1, 1);
//         }
//     }


//     // Generate white noise
//     // -----------------------------------------------------------------------------------------
//     idk::glTextureConfig noise_config = {
//         .internalformat = GL_RGBA8,
//         .format         = GL_RGBA,
//         .minfilter      = GL_NEAREST,
//         .magfilter      = GL_NEAREST,
//         .wrap_s         = GL_REPEAT,
//         .wrap_t         = GL_REPEAT,
//         .datatype       = GL_UNSIGNED_BYTE,
//         .genmipmap      = GL_FALSE
//     };

//     auto pixels = noisegen2D::white_u8(256, 256, 4);
//     m_noise = gltools::loadTexture2D(256, 256, pixels.get(), noise_config);
//     // -----------------------------------------------------------------------------------------
// }




// void
// idk::ParticleSystem::update( float dt, const glm::vec3 &view, idk::RenderQueue &queue )
// {
//     static std::vector<int> cull(128);
//     cull.clear();

//     for (auto &[id, emitter]: m_emitters)
//     {
//         int model = emitter.edesc.model_id;
//         // auto &drawlist = queue.getDrawList(model);

//         if (emitter.finished())
//         {
//             cull.push_back(id);
//         }

//         else
//         {
//             emitter.update(dt);
//         }
//     }

//     {
//         auto &program = m_program_clear;
//         program.bind();

//         for (int id: cull)
//         {
//             destroyEmitter(id);
        
//             program.set_uint("un_offset", uint32_t(EMITTER_MAX_PARTICLES * id));
//             program.dispatch(1, 1, 1);
//         }

//         gl::memoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
//     }



//     static double irrational = 0.25;
//     irrational += 0.0025 * 1.61803398875;
//     irrational = fmod(irrational, 1.0);



//     static std::vector<SSBO_ParticleDesc> descriptors;
//     descriptors.clear();

//     auto &program = m_program_update;
//     program.bind();

//     for (auto &[id, emitter]: m_emitters)
//     {
//         SSBO_ParticleDesc desc = {
//             .count           = glm::vec4(emitter.edesc.particles),

//             .origin          = glm::vec4(emitter.pdesc.origin, 1.0f),
//             .origin_rng      = glm::vec4(emitter.pdesc.origin_rng, 0.0f),

//             .velocity        = glm::vec4(emitter.pdesc.velocity, 0.0f),
//             .velocity_rng    = glm::vec4(emitter.pdesc.velocity_rng, 0.0f),

//             .scale           = glm::vec4(emitter.pdesc.scale, 0.0),
//             .scale_factor    = glm::vec4(emitter.pdesc.scale_factor, 0.0),
//             .scale_rng       = glm::vec4(emitter.pdesc.scale_rng),

//             .timer           = glm::vec4(emitter.getTimer()),
//             .duration        = glm::vec4(emitter.pdesc.duration),
//             .duration_rng    = glm::vec4(emitter.pdesc.duration_rng),
//         };

//         descriptors.push_back(desc);

//         m_SSBO_desc.bufferSubData(
//             0, sizeof(SSBO_ParticleDesc) * descriptors.size(), descriptors.data()
//         );

//         program.set_sampler2D("un_noise", 0, m_noise);
//         program.set_float("un_dtime", dt);
//         program.set_float("un_irrational", irrational);

//         program.set_uint("un_offset", uint32_t(EMITTER_MAX_PARTICLES * id));

//         program.dispatch(descriptors.size(), 1, 1);

//         descriptors.clear();
//     }

// }




// void
// idk::ParticleSystem::render( idk::ModelAllocator &MA )
// {
//     glDepthMask(GL_FALSE);
//     gl::enable(GL_BLEND);
//     gl::blendFunc(GL_SRC_ALPHA, GL_ONE);

//     static const idk::glTextureConfig config = {
//         .target         = GL_TEXTURE_2D,
//         .internalformat = GL_RGBA8,
//         .format         = GL_RGBA,
//         .minfilter      = GL_NEAREST,
//         .magfilter      = GL_LINEAR,
//         .wrap_s         = GL_REPEAT,
//         .wrap_t         = GL_REPEAT,
//         .datatype       = GL_UNSIGNED_BYTE,
//         .genmipmap      = GL_TRUE,
//         .bindless       = GL_FALSE
//     };

//     static uint32_t texture = gltools::loadTexture("assets/textures/fire.png", config);

//     static std::vector<idk::glDrawCmd> commands;
//     commands.clear();


//     auto &program = m_program_gpass;
//     program.bind();

//     for (auto &[id, emitter]: m_emitters)
//     {
//         int model_id = emitter.edesc.model_id;

//         // for (auto &mesh: MA.getModel(model_id).meshes)
//         auto &mesh = MA.getModel(model_id).meshes[0];
//         {
//             idk::glDrawCmd cmd = {
//                 .count         = mesh.numIndices,
//                 .instanceCount = uint32_t(emitter.edesc.particles),
//                 .firstIndex    = mesh.firstIndex,
//                 .baseVertex    = mesh.baseVertex,
//                 .baseInstance  = 0
//             };

//             commands.push_back(cmd);
//         }

//         auto &mat = MA.getMaterial(mesh.material);
//         program.set_sampler2D("un_albedo", 0, mat.textures[0]);
//         program.set_uint("un_offset", uint32_t(EMITTER_MAX_PARTICLES * id));

//         m_DIB.bufferSubData(0, sizeof(idk::glDrawCmd) * commands.size(), commands.data());
//         gl::bindBuffer(GL_DRAW_INDIRECT_BUFFER, m_DIB.ID());

//         gl::multiDrawElementsIndirect(
//             GL_TRIANGLES,
//             GL_UNSIGNED_INT,
//             nullptr,
//             commands.size(),
//             sizeof(idk::glDrawCmd)
//         );

//         commands.clear();
//     }

//     gl::disable(GL_BLEND);
//     glDepthMask(GL_TRUE);

// }





// ParticleSystem::Emitter::Emitter( const ParticleDesc &pd, const EmitterDesc &ed )
// :   m_timer(0.0f)
//     // m_particles(ed.particles)
// {
//     // position  = pos; // glm::vec3(T[3]);
//     // direction = dir; // glm::vec3(T * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
//     // direction = glm::normalize(direction);
//     pdesc     = pd;
//     edesc     = ed;


//     // for (Particle &p: m_particles)
//     // {
//     //     _reset_particle(p);
//     // }

// }


// // void
// // ParticleSystem::Emitter::_reset_particle( Particle &p )
// // {
// //     p.timer  = pdesc.duration_rng * idk::randf(-1.0f, +1.0f);

// //     p.pos    = position;
// //     p.pos   += pdesc.origin_rng * idk::randf(-1.0f, +1.0f);

// //     // p.vel    = direction; // glm::vec3(transform * glm::vec4(direction, 0.0f));
// //     p.vel    = pdesc.velocity; // * glm::normalize(p.vel);
// //     p.vel   += pdesc.velocity_rng * idk::randvec3(-1.0f, +1.0f);

// //     // p.scale  = pdesc.scale;
// //     // p.scale += pdesc.scale_rng * idk::randf(-1.0f, +1.0f);
// // }


// void
// ParticleSystem::Emitter::update( float dt )
// {
//     static const glm::mat4 I = glm::mat4(1.0f);

//     // for (Particle &p: m_particles)
//     // {
//     //     p.pos += dt*p.vel;

//     //     p.timer += dt;
//     //     p.scale = pdesc.scale * (p.timer / pdesc.duration);
//     //     p.scale = glm::clamp(p.scale, 0.01f, p.scale);

//     //     glm::mat4 T = glm::translate(I, p.pos);
//     //     glm::mat4 S = glm::scale(I, glm::vec3(p.scale));
//     //     glm::mat4 R = glm::inverse(
//     //         glm::lookAt(
//     //             glm::vec3(0.0f),
//     //             glm::normalize(p.pos - view),
//     //             glm::vec3(0.0f, 1.0f, 0.0f)
//     //         )
//     //     );

//     //     drawlist.push_back(T * R * S);

//     //     if (p.timer >= pdesc.duration)
//     //     {
//     //         _reset_particle(p);
//     //     }
//     // }

//     if (edesc.duration > 0.0f)
//     {
//         m_timer += dt;
//     }
// }




// int
// idk::ParticleSystem::createEmitter( const glm::vec3 &pos, const glm::vec3 &dir,
//                                     const EmitterDesc &ed, const ParticleDesc &pd )
// {
//     int id = m_emitters.create(Emitter(pos, dir, pd, ed));
//     LOG_DEBUG() << "Created particle emitter with id " << id;
//     return id;
// }



// void
// idk::ParticleSystem::destroyEmitter( int id )
// {
//     LOG_DEBUG() << "Destroyed particle emitter with id " << id;
//     m_emitters.destroy(id);
// }


// idk::ParticleSystem::Emitter&
// idk::ParticleSystem::getEmitter( int id )
// {
//     return m_emitters.get(id);
// }






// static void
// ParticleSystem_genQuadVAO()
// {
//     float quad_vertices[] = {
//       -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,
//       -1.0f, -1.0f,  0.0f,  0.0f,  0.0f,
//        1.0f, -1.0f,  0.0f,  1.0f,  0.0f,

//       -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,
//        1.0f, -1.0f,  0.0f,  1.0f,  0.0f,
//        1.0f,  1.0f,  0.0f,  1.0f,  1.0f
//     };

//     // Send screen quad to GPU
//     // ------------------------------------------------------------------------------------
//     gl::genVertexArrays(1, &m_quad_VAO);
//     gl::genBuffers(1, &m_quad_VBO);

//     gl::bindVertexArray(m_quad_VAO);
//     gl::bindBuffer(GL_ARRAY_BUFFER, m_quad_VBO);
//     gl::bufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

//     gl::vertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), 0);
//     gl::enableVertexAttribArray(0);

//     gl::vertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), 3*sizeof(float));
//     gl::enableVertexAttribArray(1);
//     // ------------------------------------------------------------------------------------
// }




