#ifndef ASSET_H
#define ASSET_H

typedef struct {
    int vertex_idx;
    int normal_idx;
    int tex_idx;
} FaceVertex;

typedef struct {
    FaceVertex vertices[16];
    int vertex_count;
} Face;

typedef struct {
    V3f *position;
    V3f *normals;
    V3f *tex;
    Face *faces;
} Asset_Model;

typedef struct {
    char *pixels;
    int width;
    int height;
} Image;

Asset_Model *load_model_from_file(const char *file);

#endif // ASSET_H
