#pragma once

#include "../renderstage/renderstage.hpp"


namespace idk
{
    class RenderStageTerrain;
    class RenderStageFoliage;
    class RenderStageWater;
}


class idk::RenderStageTerrain: public idk::RenderStage
{
private:

public:
    virtual void init( RenderEngine& ) final;
    virtual void update( RenderEngine&, Framebuffer& ) final;

};



class idk::RenderStageFoliage: public idk::RenderStage
{
private:

public:
    virtual void init( RenderEngine& ) final;
    virtual void update( RenderEngine&, Framebuffer& ) final;

};



class idk::RenderStageWater: public idk::RenderStage
{
private:

public:
    virtual void init( RenderEngine& ) final;
    virtual void update( RenderEngine&, Framebuffer& ) final;

};

