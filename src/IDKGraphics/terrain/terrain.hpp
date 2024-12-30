#pragma once

#include <libidk/idk_gl.hpp>
#include "../batching/idk_model_allocator.hpp"
#include "../storage/buffers.hpp"


namespace idk
{
    class RenderEngine;
}


namespace idk::TerrainRenderer
{
    struct TerrainDesc;

    struct SSBO_Terrain
    {
        uint64_t height;
        uint64_t nmap;
        uint64_t diff;
        uint64_t norm;
        uint64_t arm;
        uint64_t disp;
    };

    struct WaterTemp
    {
        float xscale = 2.5;
        float yscale = 0.5;
        float tscale = 2.0;
        float amp_factor = 0.52;
        float wav_factor = 1.77;
        float hoz_scale  = 8.0;
        glm::vec4 mul_factors = glm::vec4(0.0f, 0.0f, 0.0f, 1111.0f);
        int   waves  = 8;
    };


    WaterTemp &getWaterTemp();


    void init( idk::RenderEngine&, idk::ModelAllocator& );
    void update( idk::RenderEngine&, const IDK_Camera&, idk::ModelAllocator& );
    void render( idk::RenderEngine&, float dt, idk::glFramebuffer&, idk::glFramebuffer&, const IDK_Camera&, idk::ModelAllocator& );
    void renderGrass( idk::RenderEngine&, idk::glFramebuffer&, const IDK_Camera&, idk::ModelAllocator& );
    void renderShadow( idk::RenderEngine&, idk::glFramebuffer&, idk::ModelAllocator& );

    void generateTerrain();
    void generateGrass( const glm::vec3 &pos = glm::vec3(0.0f) );


    void setWaterActive( bool );
    void setTerrainWireframe( bool );
    void setWaterWireframe( bool );

    float heightQuery( float x, float z );
    glm::vec3 slopeQuery( float x, float z );

    float waterHeightQuery( float x, float z, float *dx=nullptr, float *dz=nullptr );


    void setTerrainTransform( const glm::mat4& );
    TerrainDesc &getTerrainDesc();

    uint32_t getHeightMap();
    uint32_t getNormalMap();

}


