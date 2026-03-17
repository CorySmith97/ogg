#include "assets.h"

#define BUFFER_SIZE 1024

Asset_Model *
load_model_from_file(const char *file)
{
    char *line = NULL;
    size_t len;
    ssize_t read;
    Asset_Model *model = calloc(1, sizeof(Asset_Model));
    FaceVertex *faces = NULL;

    FILE *f = fopen(file, "r"); 
    if (f == NULL)
        goto ret;

    size_t file_length = fseek(f, 0, SEEK_END);
    UNUSED(file_length);
    fseek(f, 0, SEEK_SET);
        
    while ((read = getline(&line, &len, f)) != -1) {
        if (line[0] == 'v') {
            if (line[1] == 'n') {
                V3f v = {0};
                sscanf(line, "vn %f %f %f", &v.x, &v.y, &v.z);
                arrput(model->normals, v);
            } else if (line[1] == 't') {
                V3f v = {0};
                sscanf(line, "vt %f %f %f", &v.x, &v.y, &v.z);
                arrput(model->tex, v);
            } else {
                V3f v = {0};
                sscanf(line, "v %f %f %f", &v.x, &v.y, &v.z);
                arrput(model->position, v);
            }
        }
        if (line[0] == 'f') {
            FaceVertex fv[3] = {0};
            int result = sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d", 
                    &fv[0].vertex_idx, &fv[0].tex_idx, &fv[0].normal_idx,
                    &fv[1].vertex_idx, &fv[1].tex_idx, &fv[1].normal_idx,
                    &fv[2].vertex_idx, &fv[2].tex_idx, &fv[2].normal_idx);
            Vertex v = {0};
            v.position = model->position[fv[0].vertex_idx - 1];
            v.normal   = model->normals[fv[0].normal_idx - 1];
            v.uv       = model->tex[fv[0].tex_idx - 1];
            arrput(model->vertices, v);

            v.position = model->position[fv[1].vertex_idx];
            v.normal = model->normals[fv[1].normal_idx];
            v.uv = model->tex[fv[1].tex_idx];
            arrput(model->vertices, v);

            v.position = model->position[fv[2].vertex_idx];
            v.normal = model->normals[fv[2].normal_idx];
            v.uv = model->tex[fv[2].tex_idx];
            arrput(model->vertices, v);
            UNUSED(fv);
            UNUSED(result);
        }
    }
    printf("Position Lenght = %td\n", arrlen(model->position));
    printf("Normals Lenght = %td\n", arrlen(model->normals));
    printf("Tex Lenght = %td\n", arrlen(model->tex));
    printf("Vertex Lenght = %td\n", arrlen(model->vertices));
    goto ret;


ret:
    fclose(f);
    return model;
}
