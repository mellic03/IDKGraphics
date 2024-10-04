#include "./noise.hpp"
#include "../storage/bindings.hpp"

#include <libidk/idk_gl.hpp>
#include <libidk/idk_random.hpp>
#include <libidk/idk_noisegen.hpp>
#include "../idk_noisegen.hpp"

#include <iostream>


using namespace idk;

#define TEX_W 512

struct f32bufferREE
{
    glm::vec2 data[TEX_W*TEX_W];

    glm::vec2 sampleNearest_i32( int x, int y )
    {
        x %= TEX_W;
        y %= TEX_W;

        return data[TEX_W*y + x];
    }

    glm::vec2 sampleBillinear( float u, float v )
    {
        u *= TEX_W;
        v *= TEX_W;

        float x_factor = (u - floor(u)) / (ceil(u) - floor(u));
        float y_factor = (v - floor(v)) / (ceil(v) - floor(v));

        int x = int(u) % TEX_W;
        int y = int(v) % TEX_W;

        glm::vec2 u00 = sampleNearest_i32(x+0, y+0);
        glm::vec2 u01 = sampleNearest_i32(x+1, y+0);
        glm::vec2 u10 = sampleNearest_i32(x+0, y+1);
        glm::vec2 u11 = sampleNearest_i32(x+1, y+1);

        glm::vec2 u0 = glm::mix(u00, u01, x_factor);
        glm::vec2 u1 = glm::mix(u10, u11, x_factor);

        return glm::mix(u0, u1, y_factor);
    }

};


struct NoiseGenBuffer
{
    glm::vec4 input_bounds;
    glm::vec4 output_bounds;
    glm::vec4 data[256][256];
};



struct SSBO_Noise_buffer
{
    uint64_t handles[7];
    glm::vec2 directions[16];
};


namespace
{
    f32bufferREE *m_bluenoise;
    idk::glShaderProgram m_program_sobel;
    uint32_t m_voronoi_test;

    SSBO_Noise_buffer m_buffer;
}




uint32_t do_thing( const std::string &name, const std::string &ext,
                   uint32_t w, const idk::glTextureConfig &config )
{
    static std::vector<std::string> paths;
    paths.clear();

    for (int i=0; i<8; i++)
    {
        std::string path = "IDKGE/resources/noise/" + name + "/" + std::to_string(i) + ext;
        paths.push_back(path);
    }

    return gltools::loadTexture2DArray(w, w, paths, config);
}




void
idk::internal::upload_noise()
{
    for (int i=0; i<16; i++)
    {
        m_buffer.directions[i] = idk::randvec2(0.0f, 1.0f) * 2.0f - 1.0f;
    }




    uint32_t textures[7];

    idk::glTextureConfig noise_config = {
        .internalformat = GL_RGBA8,
        .format         = GL_RGBA,
        .minfilter      = GL_LINEAR,
        .magfilter      = GL_LINEAR,
        .wrap_s         = GL_REPEAT,
        .wrap_t         = GL_REPEAT,
        .datatype       = GL_UNSIGNED_BYTE,
        .genmipmap      = GL_FALSE
    };

    m_program_sobel = idk::glShaderProgram("IDKGE/shaders/generative/sobel.comp");

    // White
    {
        auto pixels = noisegen2D::u8_whitenoise(256, 256);
        textures[0] = gltools::loadTexture2D(256, 256, pixels.get(), noise_config);
    }

    // Blue
    {
        textures[1] = gltools::loadTexture("IDKGE/resources/noise/blue/1.png", noise_config);
    }


    textures[2] = do_thing("perlin", ".png",        512, noise_config);
    textures[3] = do_thing("perlin-super", ".jpg",  512,  noise_config);
    textures[4] = do_thing("voronoi", ".jpg",       512,  noise_config);
    textures[5] = do_thing("vein", ".jpg",          512,  noise_config);
    textures[6] = do_thing("crater", ".jpg",        512,  noise_config);

    for (int i=0; i<7; i++)
    {
        m_buffer.handles[i] = gl::getTextureHandleARB(textures[i]);
        gl::makeTextureHandleResidentARB(m_buffer.handles[i]);
    }


    idk::glBufferObject<GL_SHADER_STORAGE_BUFFER> SSBO;

    SSBO.init(shader_bindings::SSBO_Noise);
    SSBO.bufferData(sizeof(m_buffer), &m_buffer, GL_STATIC_COPY);
    SSBO.bind(shader_bindings::SSBO_Noise);


    gl::memoryBarrier(GL_ALL_BARRIER_BITS);

    m_bluenoise = new f32bufferREE;

    IDK_GLCALL(
        glGetTextureImage(
            textures[1],
            0,
            GL_RG,
            GL_FLOAT,
            TEX_W*TEX_W*sizeof(glm::vec2),
            &(m_bluenoise->data[0])
        );
    )

    gl::memoryBarrier(GL_ALL_BARRIER_BITS);
}



glm::vec2
idk::noise::BlueRG( float u, float v )
{ 
    return m_bluenoise->sampleBillinear(u, v);
}


glm::vec2
idk::noise::Randvec2( int i )
{
    return m_buffer.directions[i%16];
}
