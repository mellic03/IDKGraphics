#include "idk_mesh_allocator.hpp"

#include <libidk/idk_assert.hpp>
#include <libidk/idk_vertex.hpp>
#include <libidk/GL/idk_glBindings.hpp>

#include <iostream>

idk::MeshAllocator::MeshAllocator( size_t nbytes, uint32_t vertexformat )
:
    m_vertexformat(vertexformat),
    VBO_nbytes(nbytes),
    IBO_nbytes(nbytes)
{
    auto desc = idk::VertexFormat_desc(vertexformat);
    sizeof_vertex = desc.stride; // sizeof(idk::Vertex_P_N_T_UV);

    GLuint     bindingindex = 0;
    GLbitfield buffer_flags = GL_MAP_WRITE_BIT; // | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

    gl::createBuffers(1, &VBO);
    gl::namedBufferData(VBO, VBO_nbytes, nullptr, GL_DYNAMIC_COPY);
    // gl::namedBufferStorage(VBO, VBO_nbytes, nullptr, buffer_flags);
    // VBO_ptr = reinterpret_cast<uint8_t *>(gl::mapNamedBufferRange(VBO, 0, VBO_nbytes, buffer_flags));
    // IDK_ASSERT("Error mapping idk::MeshAllocator::VBO", VBO_ptr != nullptr);

    gl::createBuffers(1, &IBO);
    gl::namedBufferData(IBO, IBO_nbytes, nullptr, GL_DYNAMIC_COPY);
    // gl::namedBufferStorage(IBO, IBO_nbytes, nullptr, buffer_flags);
    // IBO_ptr = reinterpret_cast<uint8_t *>(gl::mapNamedBufferRange(IBO, 0, IBO_nbytes, buffer_flags));
    // IDK_ASSERT("Error mapping idk::MeshAllocator::IBO", IBO_ptr != nullptr);

    gl::createVertexArrays(1, &VAO);
    gl::vertexArrayVertexBuffer(VAO, bindingindex, VBO, 0, sizeof_vertex);
    gl::vertexArrayElementBuffer(VAO, IBO);


    for (int i=0; i<desc.attribs; i++)
    {
        gl::enableVertexArrayAttrib(VAO, i);

        gl::vertexArrayAttribFormat(
            VAO,
            i,
            desc.attrib_size[i],
            desc.attrib_datatype[i],
            GL_FALSE,
            desc.attrib_offset[i]
        );

        gl::vertexArrayAttribBinding(VAO, i, bindingindex);
    }

}


idk::MeshDescriptor
idk::MeshAllocator::loadMesh( size_t nbytes_vertices, size_t nbytes_indices,
                              void *vertices, void *indices )
{
    // void *vertices_dest = vertex_alloc(nbytes_vertices);
    // void *indices_dest  = index_alloc(nbytes_indices);

    // std::memcpy(vertices_dest, vertices, nbytes_vertices);
    // std::memcpy(indices_dest,  indices,  nbytes_indices);

    gl::namedBufferSubData(VBO, VBO_cursor, nbytes_vertices, vertices);
    gl::namedBufferSubData(IBO, IBO_cursor, nbytes_indices,  indices);

    VBO_cursor += nbytes_vertices;
    IBO_cursor += nbytes_indices;

    // std::cout << "VBO: " << 100.0f * (float(VBO_cursor) / float(VBO_nbytes)) << "%%\n";
    // std::cout << "IBO: " << 100.0f * (float(IBO_cursor) / float(IBO_nbytes)) << "%%\n";

    uint32_t nvertices = uint32_t(nbytes_vertices / sizeof(idk::Vertex_P_N_T_UV));
    uint32_t nindices  = uint32_t(nbytes_indices  / sizeof(uint32_t));

    idk::MeshDescriptor desc = {
        .baseVertex  = m_basevertex,
        .firstIndex  = m_baseindex,
        .numVertices = nvertices,
        .numIndices  = nindices
    };

    m_basevertex += nvertices;
    m_baseindex  += nindices;

    return desc;
}



void
idk::MeshAllocator::clear()
{
    VBO_cursor = 0;
    IBO_cursor = 0;
}



void *
idk::MeshAllocator::vertex_alloc( size_t nbytes )
{
    VBO_cursor += nbytes;

    IDK_ASSERT(
        "[MeshAllocator::allocVertices] Ran out of vertex storage",
        VBO_cursor < VBO_nbytes
    );

    VBO_ptr += nbytes;

    return VBO_ptr - nbytes;
}


void *
idk::MeshAllocator::index_alloc( size_t nbytes )
{
    IBO_cursor += nbytes;

    IDK_ASSERT(
        "[MeshAllocator::allocIndices] Ran out of index storage",
        IBO_cursor < IBO_nbytes
    );

    IBO_ptr += nbytes;

    return IBO_ptr - nbytes;
}





void *
idk::MeshAllocator::mapVBO( GLenum access )
{
    return gl::mapNamedBuffer(VBO, access);
}


void *
idk::MeshAllocator::mapIBO( GLenum access )
{
    return gl::mapNamedBuffer(IBO, access);
}


void
idk::MeshAllocator::unmapVBO()
{
    gl::unmapNamedBuffer(VBO);
}


void
idk::MeshAllocator::unmapIBO()
{
    gl::unmapNamedBuffer(IBO);
}



