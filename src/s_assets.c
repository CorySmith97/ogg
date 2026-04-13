#define ASSET_DIR "data/"
#define BUFFER_SIZE 1024

Texture_KV     *textures = NULL;
Asset_Model_KV *assets = NULL;

b32 load_and_store_texture(const char *name, const char *file_name)
{
    Texture *val;

    val = shget(textures, name);
    if (val) return true;
    val = load_texture_from_file(file_name, false);
    shput(textures, name, val);

    return false;
}

b32 load_and_store_model(const char *name, const char *file_name)
{
    Asset_Model *val;

    val = shget(assets, name);
    if (val) return true;
    val = load_model_from_file(file_name);
    shput(assets, name, val);

    return false;
}

Texture *get_texture(const char *name)
{
    Texture *val;

    val = shget(textures, name);
    if (val) return val;
    else return NULL;
}

Asset_Model *get_model(const char *name)
{
    Asset_Model *val;

    val = shget(assets, name);
    if (val) return val;
    else return NULL;
}

void deload_model(Asset_Model *model)
{
    UNUSED(model);
}

SimpleMtl *load_material_file(const char *file)
{
    char *line = NULL;
    size_t len;
    ssize_t read;
    SimpleMtl *mtl = malloc(sizeof(SimpleMtl));

    FILE *f = fopen(file, "r");
    if (f == NULL) {
        console_write_log_alloc("Failed to load material %s", file);
        goto ret;
    }


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

    log_info("Loaded material %s", file);
    console_write_log_alloc("[Loaded material] %s", file);

ret:
    fclose(f);
    return mtl;
}

Texture *load_texture_from_file(const char *file, bool flip)
{
    Texture *tex = malloc(sizeof(Texture));

    stbi_set_flip_vertically_on_load(flip);
    u8 *data = stbi_load(file, &tex->width, &tex->height, &tex->stride, 0);
    s32 size = tex->width * tex->height * tex->stride;
    tex->data = malloc(size);
    memcpy(tex->data, data, size);

    log_info("Loaded texture  %s", file);
    console_write_log_alloc("[Loaded texture] %s", file);
    stbi_image_free(data);
    return tex;
}

Asset_Model *load_model_from_file(const char *file)
{
    char *line = NULL;
    size_t len;
    ssize_t read;
    Asset_Model *model = calloc(1, sizeof(Asset_Model));
    V3f *position = NULL;
    V3f *normals = NULL;
    V3f *tex = NULL;

    FILE *f = fopen(file, "r"); 
    if (f == NULL)
        goto ret;

    while ((read = getline(&line, &len, f)) != -1) {
        if (line[0] == 'v') {
            if (line[1] == 'n') {
                V3f v = {0};
                sscanf(line, "vn %f %f %f", &v.x, &v.y, &v.z);
                arrput(normals, v);
            } else if (line[1] == 't') {
                V3f v = {0};
                sscanf(line, "vt %f %f %f", &v.x, &v.y, &v.z);
                arrput(tex, v);
            } else {
                V3f v = {0};
                sscanf(line, "v %f %f %f", &v.x, &v.y, &v.z);
                arrput(position, v);
            }
        }
        if (line[0] == 'f') {
            FaceVertex fv[4] = {0};

            // try quad first
            s32 result = sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d", 
                    &fv[0].vertex_idx, &fv[0].tex_idx, &fv[0].normal_idx,
                    &fv[1].vertex_idx, &fv[1].tex_idx, &fv[1].normal_idx,
                    &fv[2].vertex_idx, &fv[2].tex_idx, &fv[2].normal_idx,
                    &fv[3].vertex_idx, &fv[3].tex_idx, &fv[3].normal_idx);

            s32 face_count = (result == 12) ? 4 : 3;  // quad or tri

            // always push first triangle (0, 1, 2)
            s32 indices[6] = {0, 1, 2, 0, 2, 3};
            s32 tri_count = (face_count == 4) ? 2 : 1;

            for (s32 t = 0; t < tri_count; t++) {
                for (s32 v = 0; v < 3; v++) {
                    s32 fi = indices[t * 3 + v];
                    Vertex vert = {0};
                    vert.position = position[fv[fi].vertex_idx - 1];
                    vert.normal   = normals[fv[fi].normal_idx - 1];
                    vert.uv       = tex[fv[fi].tex_idx - 1];
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
    log_info("Model loaded    %s", file);
    log_info("Vertices loaded %td", arrlen(model->vertices));
    console_write_log_alloc("Loaded model %s", file);
    goto ret;


ret:
    fclose(f);
    return model;
}


Font *load_font(const char *file, s32 cwidth, s32 cheight)
{
    Font *font = malloc(sizeof(Font));
    font->character_width = cwidth;
    font->character_height = cheight;
    font->texture = load_texture_from_file(file, false);
    return font;
}
