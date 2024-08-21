#include "./noise.hpp"
#include "../storage/bindings.hpp"

#include <libidk/idk_gl.hpp>
#include <libidk/idk_random.hpp>
#include <libidk/idk_noisegen.hpp>

#include <iostream>


using namespace idk;



struct NoiseGenBuffer
{
    glm::vec4 input_bounds;
    glm::vec4 output_bounds;
    glm::vec4 data[256][256];
};



namespace
{
    idk::glShaderProgram m_program_sobel;
    uint32_t m_voronoi_test;
}


// uint32_t idk::noise::getVoronoiTest()
// {
//     return m_voronoi_test;
// }






// static void
// generate_voronoi( int cells, int tex_w, uint32_t texture )
// {
//     static idk::glBufferObject<GL_SHADER_STORAGE_BUFFER> SSBO(
//         shader_bindings::SSBO_NoiseGen, sizeof(NoiseGenBuffer), GL_DYNAMIC_COPY
//     );

//     static NoiseGenBuffer *buffer = new NoiseGenBuffer;


//     float grid_w   = cells;
//     float cell_w = 0.5f / (tex_w / cells);

//     std::cout << grid_w << ", " << cell_w << "\n";

//     for (int row=0; row<cells; row++)
//     {
//         for (int col=0; col<cells; col++)
//         {
//             glm::vec2 texcoord = glm::vec2(col*cell_w, row*cell_w);
//                       texcoord += idk::randvec2(0.0f, cell_w);

//             float h  = idk::randf();

//             buffer->data[row][col] = glm::vec4(texcoord.x, texcoord.y, h, 0.0f);
//         }
//     }

//     buffer->input_bounds  = glm::vec4(float(grid_w));
//     buffer->output_bounds = glm::vec4(float(tex_w));

//     SSBO.bufferSubData(0, sizeof(NoiseGenBuffer), (void *)(buffer));

//     {
//         gl::bindImageTexture(0, texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R16F);

//         auto &program = m_program_voronoi;
//         program.bind();
//         program.dispatch(tex_w/8, tex_w/8, 1);

//         gl::memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
//     }

// }



uint32_t do_thing( const std::string &name, const std::string &ext,
                   uint32_t w, const idk::glTextureConfig &config )
{
    static std::vector<std::string> paths;
    paths.clear();

    for (int i=0; i<8; i++)
    {
        std::string path = "IDKGE/resources/noise/" + name + "/" + std::to_string(i) + ext;

        std::cout << path << "\n";
        paths.push_back(path);
    }

    return gltools::loadTexture2DArray(w, w, paths, config);
}




void
idk::internal::upload_noise()
{
    uint32_t textures[7];
    uint64_t handles[7];

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

    // idk::glTextureConfig voronoi_config = {
    //     .internalformat = GL_R16F,
    //     .format         = GL_RED,
    //     .minfilter      = GL_LINEAR,
    //     .magfilter      = GL_LINEAR,
    //     .wrap_s         = GL_REPEAT,
    //     .wrap_t         = GL_REPEAT,
    //     .datatype       = GL_FLOAT,
    //     .genmipmap      = GL_FALSE
    // };

    m_program_sobel = idk::glShaderProgram("IDKGE/shaders/generative/sobel.comp");
    // m_voronoi_test = gltools::loadTexture2D(512, 512, nullptr, voronoi_config);
    // generate_voronoi(32, 512, m_voronoi_test);


    // White
    {
        auto pixels = noisegen2D::u8_whitenoise(256, 256);
        textures[0] = gltools::loadTexture2D(256, 256, pixels.get(), noise_config);
    }


    // Blue
    {
        textures[1] = gltools::loadTexture("IDKGE/resources/noise/blue/0.png", noise_config);
    }


    textures[2] = do_thing("perlin", ".png",        512, noise_config);
    textures[3] = do_thing("perlin-super", ".jpg",  512,  noise_config);
    textures[4] = do_thing("voronoi", ".jpg",       512,  noise_config);
    textures[5] = do_thing("vein", ".jpg",          512,  noise_config);
    textures[6] = do_thing("crater", ".jpg",        512,  noise_config);


    for (int i=0; i<7; i++)
    {
        handles[i] = gl::getTextureHandleARB(textures[i]);
        gl::makeTextureHandleResidentARB(handles[i]);
    }


    idk::glBufferObject<GL_SHADER_STORAGE_BUFFER> SSBO;

    SSBO.init(shader_bindings::SSBO_Noise);
    SSBO.bufferData(sizeof(handles), &handles, GL_STATIC_COPY);
    SSBO.bind(shader_bindings::SSBO_Noise);

}


