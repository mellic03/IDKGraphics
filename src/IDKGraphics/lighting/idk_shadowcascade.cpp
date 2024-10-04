#include "idk_shadowcascade.hpp"



std::vector<glm::vec4>
idk::glDepthCascade::get_corners( const glm::mat4 &P, const glm::mat4 &V )
{
    std::vector<glm::vec4> corners;

    glm::mat4 inv = glm::inverse(P * V);

    for (uint x=0; x<2; ++x)
    {
        for (uint y=0; y<2; ++y)
        {
            for (uint z=0; z<2; ++z)
            {
                glm::vec4 pt = inv * glm::vec4(
                    2.0f*x - 1.0f,
                    2.0f*y - 1.0f,
                    2.0f*z - 1.0f,
                    1.0f
                );

                corners.push_back(pt / pt.w);
            }
        }
    }

    return corners;
}


glm::vec3
idk::glDepthCascade::get_center( const std::vector<glm::vec4> &corners )
{
    glm::vec3 center;
    center = glm::vec3(0.0f);

    for (const glm::vec4 &v: corners)
    {
        center += glm::vec3(v);
    }

    center /= 8.0f;

    return center;
}


glm::mat4
idk::glDepthCascade::get_view( const glm::vec3 &L, const std::vector<glm::vec4> &corners )
{
    glm::mat4 view;

    glm::vec3 center = get_center(corners);

    view = glm::lookAt(
        center - L,
        center,
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    return view;
}


#include <iostream>

glm::mat4
idk::glDepthCascade::get_projection( const float texture_width,
                                     const glm::vec3 &L,
                                     const glm::mat4 &V,
                                     const std::vector<glm::vec4> &corners )
{
    glm::mat4 P;

    glm::vec3 minv = glm::vec3(+INFINITY);
    glm::vec3 maxv = glm::vec3(-INFINITY);

    for (glm::vec4 v: corners)
    {
        v = V * v;

        minv.x = glm::min(minv.x, v.x);
        minv.y = glm::min(minv.y, v.y);
        minv.z = glm::min(minv.z, v.z);
        maxv.x = glm::max(maxv.x, v.x);
        maxv.y = glm::max(maxv.y, v.y);
        maxv.z = glm::max(maxv.z, v.z);
    }

    constexpr float zMult = 10.0f;
    minv.z *= (minv.z < 0) ? zMult : 1.0f/zMult;
    maxv.z *= (maxv.z < 0) ? 1.0f/zMult : zMult;

    P = glm::ortho(minv.x, maxv.x, minv.y, maxv.y, minv.z, maxv.z) * V;

    return P;
}




void
idk::glDepthCascade::reset( int w, int h )
{

}


void
idk::glDepthCascade::setCascadeDepths( float a, float b, float c, float d )
{
    m_cascade_depths = glm::vec4(a, b, c, d);
}


void
idk::glDepthCascade::setOutputAttachment( GLint idx )
{
    gl::namedFramebufferTextureLayer(
        m_FBO,
        GL_DEPTH_ATTACHMENT,
        depth_attachment,
        0,
        idx
    );
}


GLuint
idk::glDepthCascade::getTextureArray()
{
    return depth_attachment;
}


std::vector<glm::mat4>
idk::glDepthCascade::computeCascadeMatrices( float cam_fov,  float cam_aspect,
                                             float cam_near, float cam_far,
                                             float image_w,
                                             const glm::mat4 &cam_view,
                                             const glm::vec3 &light_dir,
                                             const glm::vec4 &cascades,
                                             const glm::vec4 &xyzmult )
{
    std::vector<glm::mat4> cascade_matrices;

    float near = cam_near;

    for (int i=0; i<NUM_CASCADES; i++)
    {
        float far = cam_near + cascades[i];
        float near = (i==0) ? cam_near : cascades[i-1];

        glm::mat4 P = glm::perspective(
            cam_fov, cam_aspect, near, far
        );

        auto corners = get_corners(P, cam_view);
        glm::mat4 V = get_view(light_dir, corners);


        glm::vec3 minv = glm::vec3(+INFINITY);
        glm::vec3 maxv = glm::vec3(-INFINITY);

        for (glm::vec4 v: corners)
        {
            glm::vec4 tmp = glm::vec4(V * v);

            minv.x = std::min(minv.x, tmp.x);
            minv.y = std::min(minv.y, tmp.y);
            minv.z = std::min(minv.z, tmp.z);
            maxv.x = std::max(maxv.x, tmp.x);
            maxv.y = std::max(maxv.y, tmp.y);
            maxv.z = std::max(maxv.z, tmp.z);
        }

        glm::vec3 M = glm::max(xyzmult, 1.0f);

        minv.x *= (minv.x < 0) ? M.x : 1.0f/M.x;
        maxv.x *= (maxv.x < 0) ? 1.0f/M.x : M.x;

        minv.y *= (minv.y < 0) ? M.x : 1.0f/M.x;
        maxv.y *= (maxv.y < 0) ? 1.0f/M.x : M.x;

        minv.z *= (minv.z < 0) ? M.x : 1.0f/M.x;
        maxv.z *= (maxv.z < 0) ? 1.0f/M.x : M.x;

        minv.z *= (minv.z < 0) ? M.z : 1.0f/M.z;
        maxv.z *= (maxv.z < 0) ? 1.0f/M.z : M.z;

        glm::mat4 proj =  glm::ortho(minv.x, maxv.x, minv.y, maxv.y, minv.z, maxv.z);
        cascade_matrices.push_back(proj * V);
    }

    return cascade_matrices;
}


