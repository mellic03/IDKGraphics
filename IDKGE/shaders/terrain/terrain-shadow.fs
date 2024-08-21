#version 460 core

#extension GL_GOOGLE_include_directive: require


in FS_in
{
    vec3 fragpos;
    vec3 normal;
    vec3 tangent;
    vec2 texcoord;
    float tesslevel;
    float height;
    mat3 TBN;

} fsin;




void main()
{

}
