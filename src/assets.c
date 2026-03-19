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

            v.position = model->position[fv[1].vertex_idx - 1];
            v.normal = model->normals[fv[1].normal_idx - 1];
            v.uv = model->tex[fv[1].tex_idx - 1];
            arrput(model->vertices, v);

            v.position = model->position[fv[2].vertex_idx - 1];
            v.normal = model->normals[fv[2].normal_idx - 1];
            v.uv = model->tex[fv[2].tex_idx - 1];
            arrput(model->vertices, v);
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


void draw_model(Asset_Model *model) 
{

    V3f rot = v3f(0, 0, 2);
    static float angle = 0;
    angle += 0.01;
    int pushed_count = 0;
     for (size_t i = 0; i < arrlen(model->vertices); i += 3) {
         Vertex v1 = model->vertices[i];
         Vertex v2 = model->vertices[i+1];
         Vertex v3 = model->vertices[i+2];
         v1.position.z = -v1.position.z;
         v2.position.z = -v2.position.z;
         v3.position.z = -v3.position.z;
         v1.position.z += 2;
         v2.position.z += 2;
         v3.position.z += 2;

        if (v1.position.z <= NEAR || v2.position.z <= NEAR || v3.position.z <= NEAR) continue;
        Color color;
        color.r = rand() % 255;
        color.g = rand() % 255;
        color.b = rand() % 255;
        color.a = 255;
        // @todo:cs move to a push triangle for the renderer to 
        // dispatch rendering to threads.
        renderer_push_triangle(
        v3f_rotate_y_around_point(v1.position, rot, angle),
        v3f_rotate_y_around_point(v2.position, rot, angle),
        v3f_rotate_y_around_point(v3.position, rot, angle),
             COLOR_PURPLE);
        pushed_count += 1;
    }

     logger(LOG_INFO, "Pushed count: %d", pushed_count);
}
