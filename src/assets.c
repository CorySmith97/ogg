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

// TODO add model translation and rotation matrices as paramters
void draw_model(Asset_Model *model, V3f position, Mat3 rotation) 
{

    static float angle = 0;
    angle += 0.01;
    int pushed_count = 0;
     for (size_t i = 0; i < arrlen(model->vertices); i += 3) {
         Vertex v1 = model->vertices[i];
         Vertex v2 = model->vertices[i+1];
         Vertex v3 = model->vertices[i+2];

         V3f p1 = v3f_mul_mat3(v1.position, rotation);
         V3f p2 = v3f_mul_mat3(v2.position, rotation);
         V3f p3 = v3f_mul_mat3(v3.position, rotation);


         p1.z = -p1.z;
         p2.z = -p2.z;
         p3.z = -p3.z;
         p1 = v3f_add(p1, position);
         p2 = v3f_add(p2, position);
         p3 = v3f_add(p3, position);

        if (p1.z <= NEAR || p2.z <= NEAR || p3.z <= NEAR) continue;

        Color color;
        color.r = (uint8_t)((v1.normal.x * 0.5f + 0.5f) * 255);
        color.g = (uint8_t)((v1.normal.y * 0.5f + 0.5f) * 255);
        color.b = (uint8_t)((v1.normal.z * 0.5f + 0.5f) * 255);
        color.a = 255;

        renderer_push_triangle(
                p1,
                p2,
                p3,
             color);
        pushed_count += 1;
    }

}
