#ifndef ASSET_H
#define ASSET_H

typedef enum
{
    OR_Regular,
    OR_Flipped,
} Orientation;

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
    int width;
    int height;
    int stride;
    char *data;
} Texture;

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
    Texture *diffuse_texture;   // map_Kd
} SimpleMtl;

typedef struct {
    V3f *position;
    V3f *normals;
    V3f *tex;
    Vertex *vertices;
    Face *faces;
    SimpleMtl *mtl;
} Asset_Model;

/* Fonts are monospaces texture maps. Each character is the same size "sprite"
 */
typedef struct {
    int width;
    int height;
    int character_width;
    int character_height;
    Orientation orientation;
    char *bitmap;
} Font;

typedef struct {
    char *pixels;
    int width;
    int height;
} Image;

typedef struct {
    char *key;
    Asset_Model *value;
} Asset_Model_KV;

Asset_Model *load_model_from_file(const char *file);
void         deload_model(Asset_Model *model);

#endif // ASSET_H
