#include "assets.h"

#define BUFFER_SIZE 1024

Asset_Model *
load_model_from_file(const char *file)
{
    char *line = NULL;
    size_t len;
    ssize_t read;
    Asset_Model *model = malloc(sizeof(Asset_Model));

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
            FaceVertex fv = {0};
            int result = sscanf(line, "f %d/%d/%d", &fv.vertex_idx, &fv.normal_idx, &fv.tex_idx);
            UNUSED(fv);
            UNUSED(result);
        }
    }
    printf("Position Lenght = %td\n", arrlen(model->position));
    printf("Normals Lenght = %td\n", arrlen(model->normals));
    printf("Tex Lenght = %td\n", arrlen(model->tex));
    goto ret;


ret:
    fclose(f);
    return model;
}
