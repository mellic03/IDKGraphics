#include "idk_model.hpp"
#include <libidk/idk_serialize.hpp>


uint32_t
idk::VertexFormat_sizeof( uint32_t format )
{
    switch (format)
    {
        using enum idk::ModelVertexFormat;

        case ModelVertexFormat_POS_NORM_TAN_UV:
            return sizeof(idk::Vertex_P_N_T_UV);
    
        case ModelVertexFormat_POS_NORM_TAN_UV_SKINNED:
            return sizeof(idk::Vertex_P_N_T_UV_SKINNED);
    }

    return 0;
}



idk::VertexFormatDescriptor
idk::VertexFormat_desc( uint32_t format )
{
    idk::VertexFormatDescriptor desc;

    int idx;
    size_t offset;


    switch (format)
    {
        using enum idk::ModelVertexFormat;

        case ModelVertexFormat_POS_NORM_TAN_UV:

            desc.stride = sizeof(idk::Vertex_P_N_T_UV);
            desc.attribs = 4;

            idx = 0;
            desc.attrib_size[idx]   = 3;
            desc.attrib_nbytes[idx] = sizeof(glm::vec3);
            idx += 1;

            desc.attrib_size[idx]   = 3;
            desc.attrib_nbytes[idx] = sizeof(glm::vec3);
            idx += 1;

            desc.attrib_size[idx]   = 3;
            desc.attrib_nbytes[idx] = sizeof(glm::vec3);
            idx += 1;

            desc.attrib_size[idx]   = 2;
            desc.attrib_nbytes[idx] = sizeof(glm::vec2);
            idx += 1;


            idx = 0;
            desc.attrib_datatype[idx] = GL_FLOAT;   idx += 1;
            desc.attrib_datatype[idx] = GL_FLOAT;   idx += 1;
            desc.attrib_datatype[idx] = GL_FLOAT;   idx += 1;
            desc.attrib_datatype[idx] = GL_FLOAT;   idx += 1;


            idx = 0;
            offset = 0;

            for (int i=0; i<4; i++)
            {
                desc.attrib_offset[i] = offset;
                offset += desc.attrib_nbytes[i];
            }

        break;


        case ModelVertexFormat_POS_NORM_TAN_UV_SKINNED:

            desc.stride = sizeof(idk::Vertex_P_N_T_UV_SKINNED);
            desc.attribs = 6;

            idx = 0;
            desc.attrib_size[idx]   = 3;
            desc.attrib_nbytes[idx] = sizeof(glm::vec3);
            idx += 1;

            desc.attrib_size[idx]   = 3;
            desc.attrib_nbytes[idx] = sizeof(glm::vec3);
            idx += 1;

            desc.attrib_size[idx]   = 3;
            desc.attrib_nbytes[idx] = sizeof(glm::vec3);
            idx += 1;

            desc.attrib_size[idx]   = 2;
            desc.attrib_nbytes[idx] = sizeof(glm::vec2);
            idx += 1;

            desc.attrib_size[idx]   = 4;
            desc.attrib_nbytes[idx] = sizeof(glm::ivec4);
            idx += 1;

            desc.attrib_size[idx]   = 4;
            desc.attrib_nbytes[idx] = sizeof(glm::vec4);
            idx += 1;


            idx = 0;
            desc.attrib_datatype[idx] = GL_FLOAT;   idx += 1;
            desc.attrib_datatype[idx] = GL_FLOAT;   idx += 1;
            desc.attrib_datatype[idx] = GL_FLOAT;   idx += 1;
            desc.attrib_datatype[idx] = GL_FLOAT;   idx += 1;
            desc.attrib_datatype[idx] = GL_INT;     idx += 1;
            desc.attrib_datatype[idx] = GL_FLOAT;   idx += 1;


            idx = 0;
            offset = 0;

            for (int i=0; i<6; i++)
            {
                desc.attrib_offset[i] = offset;
                offset += desc.attrib_nbytes[i];
            }

        break;
    }

    return desc;
}



bool
idk::MeshFile_hasTexture( uint32_t bitmask, int idx )
{
    return (bitmask & (1 << idx)) != 0;
}


uint32_t
idk::MeshFile_packBitmask( const idk::MeshFileHeader &header )
{
    uint32_t bitmask = 0;

    for (int i=0; i<draw_buffer::TEXTURES_PER_MATERIAL; i++)
    {
        if (header.textures[i] != "")
        {
            bitmask |= (1 << i);
        }
    }

    return bitmask;
}


idk::MeshFileHeader
idk::MeshFile_new()
{
    idk::MeshFileHeader header;

    for (int i=0; i<draw_buffer::TEXTURES_PER_MATERIAL; i++)
    {
        header.textures[i] = "";
    }

    return header;
}



void
idk::MeshFile_write( std::ofstream          &stream,
                     uint32_t                vertexformat,
                     const MeshFileHeader   &header,
                     const void             *vertices,
                     const void             *indices )
{
    stream.write(reinterpret_cast<const char *>(&header), 4*sizeof(uint32_t));

    // idk_printvalue(header.bounding_radius);
    // idk_printvalue(header.num_verts);
    // idk_printvalue(header.num_indices);
    // idk_printvalue(header.bitmask);

    for (int i=0; i<draw_buffer::TEXTURES_PER_MATERIAL; i++)
    {
        if (MeshFile_hasTexture(header.bitmask, i))
        {
            idk::streamwrite<std::string>(stream, header.textures[i]);
        }
    }

    uint32_t nbytes_vertices = header.num_verts * VertexFormat_sizeof(vertexformat);
    uint32_t nbytes_indices  = header.num_indices * sizeof(uint32_t);

    stream.write(reinterpret_cast<const char *>(vertices), nbytes_vertices);
    stream.write(reinterpret_cast<const char *>(indices),  nbytes_indices);
}


void
idk::MeshFile_read( std::ifstream   &stream,
                    uint32_t         vertexformat,
                    MeshFileHeader  &header,
                    void           *&vertices,
                    void           *&indices )
{

    stream.read(reinterpret_cast<char *>(&header), 4*sizeof(uint32_t));

    // idk_printvalue(header.bounding_radius);
    // idk_printvalue(header.num_verts);
    // idk_printvalue(header.num_indices);
    // idk_printvalue(header.bitmask);


    for (int i=0; i<draw_buffer::TEXTURES_PER_MATERIAL; i++)
    {
        if (MeshFile_hasTexture(header.bitmask, i))
        {
            idk::streamread<std::string>(stream, header.textures[i]);
        }
    }

    uint32_t nbytes_vertices = header.num_verts * VertexFormat_sizeof(vertexformat);
    uint32_t nbytes_indices  = header.num_indices * sizeof(uint32_t);

    vertices = std::malloc(nbytes_vertices);
    indices  = std::malloc(nbytes_indices);

    stream.read(reinterpret_cast<char *>(vertices), nbytes_vertices);
    stream.read(reinterpret_cast<char *>(indices),  nbytes_indices);

}



void
idk::ModelFile_write( std::ofstream                     &stream,
                      const idk::ModelFileHeader        &header,
                      const std::vector<MeshFileHeader> &meshes,
                      const std::vector<void *>         &vertices,
                      const std::vector<void *>         &indices )
{
    stream.write(reinterpret_cast<const char *>(&header), sizeof(ModelFileHeader));

    for (uint32_t i=0; i<meshes.size(); i++)
    {
        idk::MeshFile_write(stream, header.vertexformat, meshes[i], vertices[i], indices[i]);
    }
}



void
idk::ModelFile_read( const std::string           &filepath,
                     idk::ModelFileHeader        &header,
                     std::vector<MeshFileHeader> &meshes,
                     std::vector<void *>         &vertices,
                     std::vector<void *>         &indices )
{
    std::ifstream stream(filepath, std::ios::binary);
    
    stream.read(reinterpret_cast<char *>(&header), sizeof(idk::ModelFileHeader));
    meshes.resize(header.num_meshes);
    vertices.resize(header.num_meshes);
    indices.resize(header.num_meshes);

    for (uint32_t i=0; i<header.num_meshes; i++)
    {
        idk::MeshFile_read(stream, header.vertexformat, meshes[i], vertices[i], indices[i]);
    }

    stream.close();
}

