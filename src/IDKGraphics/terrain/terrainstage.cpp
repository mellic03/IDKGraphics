#include "terrainstage.hpp"
#include "../idk_renderengine.hpp"




void
idk::RenderStageTerrain::init( idk::RenderEngine &ren )
{
    ren.createFramebuffer(1, 1, 1);
}


void
idk::RenderStageTerrain::update( idk::RenderEngine &ren, idk::Framebuffer &outbuffer )
{

}




void
idk::RenderStageFoliage::init( idk::RenderEngine &ren )
{

}


void
idk::RenderStageFoliage::update( idk::RenderEngine &ren, idk::Framebuffer &outbuffer )
{

}




void
idk::RenderStageWater::init( idk::RenderEngine &ren )
{

}


void
idk::RenderStageWater::update( idk::RenderEngine &ren, idk::Framebuffer &outbuffer )
{

}



