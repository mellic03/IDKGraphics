
#define IDK_MAX_MATERIALS 128
#define IDK_MATERIAL_NUM_TEXTURES 5


#define IDK_MODELDATA_MAX_TEXTURES   1024
#define IDK_MODELDATA_MAX_MATERIALS  256
#define IDK_MODELDATA_MAX_TRANSFORMS 256

struct IDK_ModelData
{
    sampler2D   materials       [IDK_MODELDATA_MAX_MATERIALS][8];
    mat4        transforms      [IDK_MODELDATA_MAX_TRANSFORMS];
};


layout (std430, binding = 1) buffer SSBO_ModelData
{
    IDK_ModelData un_ModelData;
};

