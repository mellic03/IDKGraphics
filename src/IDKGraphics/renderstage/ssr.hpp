#pragma once

#include "../renderstage/renderstage.hpp"


namespace idk
{
    class RenderStageSSR;
    class RenderStageBlueTint;
}


class idk::RenderStageSSR: public idk::RenderStage
{
private:
    int m_framebufferID;

public:
    virtual void init( RenderEngine& ) final;
    virtual void update( RenderEngine&, Framebuffer& ) final;

};


class idk::RenderStageBlueTint: public idk::RenderStage
{
private:
    int m_framebufferID;

public:
    virtual void init( RenderEngine& ) final;
    virtual void update( RenderEngine&, Framebuffer& ) final;

};
