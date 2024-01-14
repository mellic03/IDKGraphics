#pragma once

#include "idk_model_handle.hpp"


namespace idk
{
    static constexpr size_t KILO = 1024;
    static constexpr size_t MEGA = 1024*1024;

    class BatchAllocator;
};


class idk::BatchAllocator
{
private:
    uint32_t m_vertexformat;
    uint32_t sizeof_vertex;
    uint32_t m_basevertex = 0;
    uint32_t m_baseindex  = 0;

    GLuint VAO;
    GLuint VBO;
    GLuint IBO;

    void  *VBO_ptr;
    void  *IBO_ptr;

    size_t VBO_nbytes;
    size_t IBO_nbytes;

    size_t VBO_cursor = 0;
    size_t IBO_cursor = 0;

public:
                        BatchAllocator( size_t nbytes, uint32_t vertexformat );

    idk::ModelHandle    load( size_t nbytes_vertices, size_t nbytes_indices,
                              void *vertices, void *indices );

    void *              allocVertices( size_t nbytes );
    void *              allocIndices( size_t nbytes );

};