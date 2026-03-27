#include "assets.h"

#define ASSET_DIR "data/"
#define BUFFER_SIZE 1024

SimpleMtl *load_material_file(const char *file)
{
    char *line = NULL;
    size_t len;
    ssize_t read;
    SimpleMtl *mtl = malloc(sizeof(SimpleMtl));

    FILE *f = fopen(file, "r");
    if (f == NULL) 
        goto ret;

    while((read = getline(&line, &len, f)) != -1) {
        if (line[0] == 'K') {
            if (line[1] == 'a') {
                sscanf(line, "Ka %f %f %f", &mtl->ambient.x, &mtl->ambient.y, &mtl->ambient.z);
            }
            if (line[1] == 'd') {
                sscanf(line, "Kd %f %f %f", &mtl->diffuse.x, &mtl->diffuse.y, &mtl->diffuse.z);
            }
        }

        if (strncmp(line, "map_Kd", 6) == 0) {
            line[strcspn(line, "\n")] = '\0';
            char file_name[256];
            snprintf(file_name, sizeof(file_name), "data/%s",  &line[7]);
            mtl->diffuse_texture = load_texture_from_file(file_name, true);
        }
    }

ret:
    fclose(f);
    return mtl;
}

Texture *load_texture_from_file(const char *file, bool flip)
{
    Texture *tex = malloc(sizeof(Texture));

    stbi_set_flip_vertically_on_load(flip);
    unsigned char *data = stbi_load(file, &tex->width, &tex->height, &tex->stride, 0);
    int size = tex->width * tex->height * tex->stride;
    tex->data = malloc(size);
    memcpy(tex->data, data, size);

    stbi_image_free(data);
    return tex;
}

Asset_Model *load_model_from_file(const char *file)
{
    char *line = NULL;
    size_t len;
    ssize_t read;
    Asset_Model *model = calloc(1, sizeof(Asset_Model));

    FILE *f = fopen(file, "r"); 
    if (f == NULL)
        goto ret;

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
            FaceVertex fv[4] = {0};

            // try quad first
            int result = sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d", 
                    &fv[0].vertex_idx, &fv[0].tex_idx, &fv[0].normal_idx,
                    &fv[1].vertex_idx, &fv[1].tex_idx, &fv[1].normal_idx,
                    &fv[2].vertex_idx, &fv[2].tex_idx, &fv[2].normal_idx,
                    &fv[3].vertex_idx, &fv[3].tex_idx, &fv[3].normal_idx);

            int face_count = (result == 12) ? 4 : 3;  // quad or tri

            // always push first triangle (0, 1, 2)
            int indices[6] = {0, 1, 2, 0, 2, 3};
            int tri_count = (face_count == 4) ? 2 : 1;

            for (int t = 0; t < tri_count; t++) {
                for (int v = 0; v < 3; v++) {
                    int fi = indices[t * 3 + v];
                    Vertex vert = {0};
                    vert.position = model->position[fv[fi].vertex_idx - 1];
                    vert.normal   = model->normals[fv[fi].normal_idx - 1];
                    vert.uv       = model->tex[fv[fi].tex_idx - 1];
                    arrput(model->vertices, vert);
                }
            }

        }
        if (line[0] == 'm') {
            if (strncmp(line, "mtllib", 6) == 0) {
                line[strcspn(line, "\n")] = '\0';
                char file_name[256];
                snprintf(file_name, sizeof(file_name), "data/%s",  &line[7]);
                model->mtl = load_material_file(file_name);
            }
        }
        if (line[0] == 'u') {
            if (strncmp(line, "usemtl", 6) == 1){}
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


Font *load_font(const char *file, int cwidth, int cheight)
{
    Font *font = malloc(sizeof(Font));
    font->character_width = cwidth;
    font->character_height = cheight;
    font->texture = load_texture_from_file(file, true);
    return font;
}
