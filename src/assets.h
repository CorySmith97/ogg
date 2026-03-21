#ifndef ASSET_H
#define ASSET_H

typedef struct Vertex {
    V3f position;
    V3f normal;
    V3f uv;
} Vertex;

typedef struct {
    int vertex_idx;
    int normal_idx;
    int tex_idx;
} FaceVertex;

typedef struct {
    FaceVertex vertices[16];
    int vertex_count;
} Face;

typedef enum
{
    IM_ColorOnAmbientOff,
    IM_ColorOnAmbientOn,
    IM_HighlightOn,
    IM_ReflectionOnRayTraceOn,
    IM_Transparency,
    IM_ReflectionFresOnRayTraceOn,
    IM_TransparencyRefractFresOffRayTraceOn,
    IM_TransparencyRefractFresOnRayTraceOn,
    IM_ReflectionRayTraceOff,
    IM_TransparencyGlass,
    IM_CastShadows,
    IM_Count,
} IlluminationModals;

typedef struct {
    V3f ambient;                // Ka
    V3f diffuse;                // Kd
    V3f specular;               // Ks
    float specular_exponent;    // Ns
    float dissolved;            // d
    float invert_dissolved;     // Tr
    V3f trans_filter_color;     // Tf
    float optical_density;      // Ni
    IlluminationModals illum;   // illum
    const char *texture_name;   // Not doing this quite yet.
} SimpleMtl;

typedef struct {
    int width;
    int height;
    char *pixels;
} Texture;

typedef struct {
    V3f *position;
    V3f *normals;
    V3f *tex;
    Vertex *vertices;
    Face *faces;
    SimpleMtl *mtl;
} Asset_Model;

typedef struct {
    char *pixels;
    int width;
    int height;
} Image;

Asset_Model *load_model_from_file(const char *file);
void         deload_model(Asset_Model *model);

#endif // ASSET_H
