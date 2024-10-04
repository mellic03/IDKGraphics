#include "ssr.hpp"
#include "../idk_renderengine.hpp"


void
idk::RenderStageSSR::init( idk::RenderEngine &ren )
{
    // idk::glTextureConfig SSR_config = {
    //     .internalformat = GL_RGBA16F,
    //     .format         = GL_RGBA,
    //     .minfilter      = GL_LINEAR,
    //     .magfilter      = GL_LINEAR,
    //     .datatype       = GL_FLOAT,
    //     .genmipmap      = GL_FALSE
    // };


    // m_framebufferID = ren.createFramebuffer(0.5f, 0.5f, 1);

    // auto &fb = ren.getFramebuffer(m_framebufferID);
    //       fb.addAttachment(0, new idk::FramebufferAttachment(SSR_config));
}


void
idk::RenderStageSSR::update( idk::RenderEngine &ren, idk::Framebuffer &outbuffer )
{
    // std::cout << "[RenderStageSSR::update]: m_framebufferID = " << m_framebufferID << "\n";

    // auto &fb = ren.getFramebuffer(m_framebufferID);

    // {
    //     auto &program = ren.getBindProgram("SSR-downsample");

    //     for (int i=0; i<6; i++)
    //     {
    //         uint32_t src = (i == 0) ? outbuffer.attachments[0] : fb.m_textures[i-1];

    //         program.set_sampler2D("un_input", src);
    
    //         gl::bindImageTexture(
    //             1, fb.m_textures[i], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F
    //         );

    //         program.set_int("un_miplevel", i);
    //         program.dispatch(width()/8, height()/8, 1);
    //         gl::memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    //     }
    // }

    // {
    //     outbuffer.bind();
    //     gl::enable(GL_BLEND);
    //     gl::blendFunc(GL_SRC_ALPHA, GL_ONE);


    //     auto &program = ren.getBindProgram("SSR");
    
    //     for (int i=0; i<6; i++)
    //     {
    //         std::string label = "un_input[" + std::to_string(i) + "]";
    //         program.set_sampler2D(label, fb.m_textures[i]);
    //     }

    //     auto &G = ren.getGBuffer();

    //     program.set_sampler2D("un_albedo", G.attachments[0]);
    //     program.set_sampler2D("un_normal", G.attachments[1]);
    //     program.set_sampler2D("un_pbr",    G.attachments[2]);
    //     program.set_sampler2D("un_fragdepth", G.attachments[3]);
    //     program.set_sampler2D("un_BRDF_LUT",  BRDF_LUT);
    //     program.set_samplerCube("un_skybox",  skyboxes[current_skybox]);

    //     gl::drawArrays(GL_TRIANGLES, 0, 6);
    // }

}






void
idk::RenderStageBlueTint::init( idk::RenderEngine &ren )
{
    // idk::glTextureConfig config = {
    //     .internalformat = GL_RGBA16F,
    //     .format         = GL_RGBA,
    //     .minfilter      = GL_LINEAR,
    //     .magfilter      = GL_LINEAR,
    //     .datatype       = GL_FLOAT,
    //     .genmipmap      = GL_FALSE
    // };


    // m_framebufferID = ren.createFramebuffer(0.5f, 0.5f, 1);

    // auto &fb = ren.getFramebuffer(m_framebufferID);
    //       fb.addAttachment(0, new idk::FramebufferAttachment(config));
}


void
idk::RenderStageBlueTint::update( idk::RenderEngine &ren, idk::Framebuffer &outbuffer )
{
    // std::cout << "RenderStageBlueTint::update\n";
}
