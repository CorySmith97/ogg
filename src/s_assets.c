b32 load_and_store_texture(const char *name, const char *file_name);
b32 load_and_store_model(const char *name, const char *file_name);

#define ASSET_DIR "data/"
#define BUFFER_SIZE 1024

Texture_KV     *textures    = NULL;
Asset_Model_KV *assets      = NULL;
GLTF_Model_KV  *gltf_assets = NULL;

// @todo:cs there will be a need for this to be moved to a String8 hashtable in the
// future.
void load_asset_catelog(void)
{
    // GLTF MODELS
    load_and_store_gltf_model("tree", "data/gltf/tree.gltf");
    load_and_store_gltf_model("grasstile", "data/gltf/grass.gltf");
    load_and_store_gltf_model("arrow", "data/gltf/arrow.gltf");
    load_and_store_gltf_model("ci", "data/gltf/unnamed_2.gltf");

    // OBJ MODELS
    load_and_store_model("shopkeeper", "data/shopkeeper.obj");
    load_and_store_model("fence", "data/lowpoly/OBJ/SM_Bld_Fence_01_Snow.obj");

    // TEXTURES
    load_and_store_texture("target", "data/target.png");
}

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

void load_and_store_gltf_model(const char *name, const char *path)
{
    if (shget(gltf_assets, name)) return;
    GLTF_Model *val = load_gltf_model(path);
    if (val) shput(gltf_assets, name, val);
}

GLTF_Model *get_gltf_model(const char *name)
{
    return shget(gltf_assets, name);
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
            //memcpy(mtl->texture_name, file_name, strlen(file_name));
        }
    }

    log_info("Loaded material %s", file);
    console_write_log_alloc("[Loaded material] %s", file);

ret:
    fclose(f);
    return mtl;
}

static void texture_upload_gl(Texture *tex)
{
    GLenum fmt = (tex->stride == 4) ? GL_RGBA
               : (tex->stride == 1) ? GL_RED
               : GL_RGB;

    glGenTextures(1, &tex->id);
    glBindTexture(GL_TEXTURE_2D, tex->id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, fmt,
                 tex->width, tex->height, 0,
                 fmt, GL_UNSIGNED_BYTE, tex->data);
    glGenerateMipmap(GL_TEXTURE_2D);
}

Texture *load_texture_from_file(const char *file, bool flip)
{
    Texture *tex = malloc(sizeof(Texture));

    stbi_set_flip_vertically_on_load(flip);
    u8 *data = stbi_load(file, &tex->width, &tex->height, &tex->stride, 0);
    s32 size = tex->width * tex->height * tex->stride;
    tex->data = malloc(size);
    memcpy(tex->data, data, size);
    stbi_image_free(data);

    texture_upload_gl(tex);

    log_info("Loaded texture  %s", file);
    console_write_log_alloc("[Loaded texture] %s", file);
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
    // Build base_positions (unique rest-pose positions) and index_buffer.
    // This is the foundation for animation skinning — the renderer always reads
    // model->vertices, but the animation system works on the smaller base set
    // and scatters results back via index_buffer.
    {
        u32 flat_count = (u32)arrlen(model->vertices);
        model->index_buffer = malloc(sizeof(u32) * flat_count);

        for (u32 j = 0; j < flat_count; j++) {
            V3f pos = model->vertices[j].position;

            u32 found = UINT32_MAX;
            for (u32 k = 0; k < (u32)arrlen(model->base_positions); k++) {
                if (v3f_equal(model->base_positions[k], pos)) {
                    found = k;
                    break;
                }
            }

            if (found == UINT32_MAX) {
                found = (u32)arrlen(model->base_positions);
                arrput(model->base_positions, pos);
            }

            model->index_buffer[j] = found;
        }

        model->base_count  = (u32)arrlen(model->base_positions);
        model->skin_groups = malloc(sizeof(u16) * model->base_count);
        for (u32 i = 0; i < model->base_count; i++)
            model->skin_groups[i] = ANIM_GROUP_STATIC;
    }

    log_info("Model loaded    %s", file);
    log_info("Vertices loaded %td", arrlen(model->vertices));
    log_info("Base positions  %u (from %td flat)", model->base_count, arrlen(model->vertices));
    console_write_log_alloc("Loaded model %s", file);
    goto ret;


ret:
    fclose(f);
    return model;
}

void skin_save(const char *file, Asset_Model *model)
{
    FILE *f = fopen(file, "w");
    if (!f) {
        console_write_log_alloc("skin_save: could not open %s", file);
        return;
    }
    fprintf(f, "%u\n", model->base_count);
    for (u32 i = 0; i < model->base_count; i++)
        fprintf(f, "%u\n", (unsigned)model->skin_groups[i]);
    fclose(f);
    console_write_log_alloc("Saved skin -> %s", file);
}

void skin_load(const char *file, Asset_Model *model)
{
    FILE *f = fopen(file, "r");
    if (!f) {
        console_write_log_alloc("skin_load: could not open %s", file);
        return;
    }
    u32 base_count = 0;
    fscanf(f, "%u\n", &base_count);
    if (base_count != model->base_count) {
        console_write_log_alloc("skin_load: count mismatch %u vs %u in %s",
                                base_count, model->base_count, file);
        fclose(f);
        return;
    }
    for (u32 i = 0; i < model->base_count; i++) {
        unsigned g = ANIM_GROUP_STATIC;
        fscanf(f, "%u\n", &g);
        model->skin_groups[i] = (u16)g;
    }
    fclose(f);
    console_write_log_alloc("Loaded skin <- %s", file);
}

Font *load_font(const char *file, s32 cwidth, s32 cheight)
{
    Font *font = malloc(sizeof(Font));
    font->character_width = cwidth;
    font->character_height = cheight;
    font->texture = load_texture_from_file(file, false);
    return font;
}

Font *load_sdf_font(const char *png_path, const char *bin_path)
{
    Font *f = malloc(sizeof(Font));

    f->texture = load_texture_from_file(png_path, false);  // your existing loader, needs GL_LINEAR

    FILE *fp = fopen(bin_path, "rb");
    u32 num_glyphs, atlas_w, atlas_h, font_size, baked_ascent;
    fread(&num_glyphs, sizeof(u32), 1, fp);
    fread(&atlas_w,    sizeof(u32), 1, fp);
    fread(&atlas_h,    sizeof(u32), 1, fp);
    fread(&font_size,  sizeof(u32), 1, fp);
    fread(&baked_ascent,sizeof(u32), 1, fp);  // add this
    fread(f->glyphs,   sizeof(GlyphInfo), num_glyphs, fp);
    fclose(fp);

    f->ascent = (f32)baked_ascent;
    f->size = (f32)font_size;
    return f;
}

static Texture *load_texture_from_memory(const u8 *buf, int len)
{
    Texture *tex = malloc(sizeof(Texture));
    stbi_set_flip_vertically_on_load(false);
    u8 *data = stbi_load_from_memory(buf, len, &tex->width, &tex->height, &tex->stride, 0);
    if (!data) { free(tex); return NULL; }
    int size = tex->width * tex->height * tex->stride;
    tex->data = malloc(size);
    memcpy(tex->data, data, size);
    stbi_image_free(data);
    texture_upload_gl(tex);
    return tex;
}

static Texture *gltf_load_texture(cgltf_texture_view *view, const char *gltf_path)
{
    if (!view->texture || !view->texture->image) return NULL;
    cgltf_image *img = view->texture->image;

    if (img->buffer_view) {
        const u8 *buf = (const u8 *)img->buffer_view->buffer->data + img->buffer_view->offset;
        return load_texture_from_memory(buf, (int)img->buffer_view->size);
    }

    if (img->uri) {
        char path[512];
        const char *last_slash = strrchr(gltf_path, '/');
        if (last_slash)
            snprintf(path, sizeof(path), "%.*s%s", (int)(last_slash - gltf_path + 1), gltf_path, img->uri);
        else
            snprintf(path, sizeof(path), "%s", img->uri);
        return load_texture_from_file(path, false);
    }

    return NULL;
}

static SimpleMtl *gltf_load_material(cgltf_material *mat, const char *gltf_path)
{
    SimpleMtl *mtl = calloc(1, sizeof(SimpleMtl));
    mtl->diffuse = v3f(1.f, 1.f, 1.f);
    mtl->ambient = v3f(0.2f, 0.2f, 0.2f);

    if (!mat) return mtl;

    if (mat->has_pbr_metallic_roughness) {
        cgltf_pbr_metallic_roughness *pbr = &mat->pbr_metallic_roughness;
        mtl->diffuse           = v3f(pbr->base_color_factor[0], pbr->base_color_factor[1], pbr->base_color_factor[2]);
        mtl->specular_exponent = (1.f - pbr->roughness_factor) * 128.f;
        mtl->diffuse_texture   = gltf_load_texture(&pbr->base_color_texture, gltf_path);
    }

    return mtl;
}

static u16 dominant_joint(float *joints, float *weights, u32 src)
{
    u16  best_j = ANIM_GROUP_STATIC;
    float best_w = -1.f;
    for (u32 wi = 0; wi < 4; wi++) {
        float wt = weights[src*4 + wi];
        if (wt > best_w) { best_w = wt; best_j = (u16)joints[src*4 + wi]; }
    }
    return best_j;
}

static Asset_Model *gltf_build_primitive(cgltf_primitive *prim, const char *gltf_path, cgltf_skin *skin)
{
    cgltf_accessor *pos_acc     = NULL;
    cgltf_accessor *norm_acc    = NULL;
    cgltf_accessor *uv_acc      = NULL;
    cgltf_accessor *joints_acc  = NULL;
    cgltf_accessor *weights_acc = NULL;

    for (cgltf_size ai = 0; ai < prim->attributes_count; ai++) {
        cgltf_attribute *attr = &prim->attributes[ai];
        if (attr->type == cgltf_attribute_type_position                        ) pos_acc     = attr->data;
        if (attr->type == cgltf_attribute_type_normal                          ) norm_acc    = attr->data;
        if (attr->type == cgltf_attribute_type_texcoord  && attr->index == 0   ) uv_acc      = attr->data;
        if (attr->type == cgltf_attribute_type_joints    && attr->index == 0   ) joints_acc  = attr->data;
        if (attr->type == cgltf_attribute_type_weights   && attr->index == 0   ) weights_acc = attr->data;
    }

    if (!pos_acc) return NULL;

    u32 vert_count = (u32)pos_acc->count;

    float *positions = malloc(sizeof(float) * 3 * vert_count);
    float *normals   = norm_acc    ? malloc(sizeof(float) * 3 * vert_count) : NULL;
    float *uvs       = uv_acc      ? malloc(sizeof(float) * 2 * vert_count) : NULL;
    float *joints    = joints_acc  ? malloc(sizeof(float) * 4 * vert_count) : NULL;
    float *weights   = weights_acc ? malloc(sizeof(float) * 4 * vert_count) : NULL;

    cgltf_accessor_unpack_floats(pos_acc, positions, 3 * vert_count);
    if (norm_acc)    cgltf_accessor_unpack_floats(norm_acc,    normals,  3 * vert_count);
    if (uv_acc)      cgltf_accessor_unpack_floats(uv_acc,      uvs,      2 * vert_count);
    if (joints_acc)  cgltf_accessor_unpack_floats(joints_acc,  joints,   4 * vert_count);
    if (weights_acc) cgltf_accessor_unpack_floats(weights_acc, weights,  4 * vert_count);

    b32 has_skin = joints && weights;

    Asset_Model *model       = calloc(1, sizeof(Asset_Model));
    u16         *skin_flat   = NULL;

    if (prim->indices) {
        u32  idx_count = (u32)prim->indices->count;
        u32 *indices   = malloc(sizeof(u32) * idx_count);
        cgltf_accessor_unpack_indices(prim->indices, indices, sizeof(u32), idx_count);

        for (u32 i = 0; i < idx_count; i++) {
            u32 idx = indices[i];
            Vertex v = {0};
            v.position = v3f(positions[idx*3+0], positions[idx*3+1], positions[idx*3+2]);
            if (normals) v.normal = v3f(normals[idx*3+0], normals[idx*3+1], normals[idx*3+2]);
            if (uvs)     v.uv     = v3f(uvs[idx*2+0],     uvs[idx*2+1],     0.f);
            arrput(model->vertices, v);
            arrput(skin_flat, has_skin ? dominant_joint(joints, weights, idx) : (u16)ANIM_GROUP_STATIC);
        }
        free(indices);
    } else {
        for (u32 i = 0; i < vert_count; i++) {
            Vertex v = {0};
            v.position = v3f(positions[i*3+0], positions[i*3+1], positions[i*3+2]);
            if (normals) v.normal = v3f(normals[i*3+0], normals[i*3+1], normals[i*3+2]);
            if (uvs)     v.uv     = v3f(uvs[i*2+0],     uvs[i*2+1],     0.f);
            arrput(model->vertices, v);
            arrput(skin_flat, has_skin ? dominant_joint(joints, weights, i) : (u16)ANIM_GROUP_STATIC);
        }
    }

    free(positions);
    if (normals)  free(normals);
    if (uvs)      free(uvs);
    if (joints)   free(joints);
    if (weights)  free(weights);

    u32 flat_count = (u32)arrlen(model->vertices);
    model->index_buffer = malloc(sizeof(u32) * flat_count);

    for (u32 j = 0; j < flat_count; j++) {
        V3f pos   = model->vertices[j].position;
        u32 found = UINT32_MAX;
        for (u32 k = 0; k < (u32)arrlen(model->base_positions); k++) {
            if (v3f_equal(model->base_positions[k], pos)) { found = k; break; }
        }
        if (found == UINT32_MAX) {
            found = (u32)arrlen(model->base_positions);
            arrput(model->base_positions, pos);
        }
        model->index_buffer[j] = found;
    }

    model->base_count  = (u32)arrlen(model->base_positions);
    model->skin_groups = malloc(sizeof(u16) * model->base_count);
    for (u32 i = 0; i < model->base_count; i++)
        model->skin_groups[i] = ANIM_GROUP_STATIC;
    for (u32 j = 0; j < flat_count; j++) {
        u32 base = model->index_buffer[j];
        if (model->skin_groups[base] == ANIM_GROUP_STATIC)
            model->skin_groups[base] = skin_flat[j];
    }
    arrfree(skin_flat);

    if (skin && skin->inverse_bind_matrices) {
        for (u32 i = 0; i < model->base_count; i++) {
            u16 g = model->skin_groups[i];
            if (g == ANIM_GROUP_STATIC) continue;
            float raw[16];
            cgltf_accessor_read_float(skin->inverse_bind_matrices, g, raw, 16);
            // GLTF is column-major; engine v4f_mul_mat4 is row-major (v*M), so transpose.
            Mat4 inv_bind = { .c = {
                raw[0], raw[4], raw[8],  raw[12],
                raw[1], raw[5], raw[9],  raw[13],
                raw[2], raw[6], raw[10], raw[14],
                raw[3], raw[7], raw[11], raw[15],
            }};
            model->base_positions[i] = v3f_translate_by_mat4(model->base_positions[i], inv_bind);
        }
    }

    model->mtl    = gltf_load_material(prim->material, gltf_path);
    model->loaded = true;

    return model;
}

static s32 gltf_find_joint(cgltf_skin *skin, cgltf_node *node)
{
    for (cgltf_size i = 0; i < skin->joints_count; i++)
        if (skin->joints[i] == node) return (s32)i;
    return -1;
}

static FrameBase *gltf_build_frame_base(cgltf_skin *skin)
{
    FrameBase *base = calloc(1, sizeof(FrameBase));
    for (cgltf_size i = 0; i < skin->joints_count; i++) {
        cgltf_node *node = skin->joints[i];
        s16 parent = -1;
        if (node->parent) {
            s32 pi = gltf_find_joint(skin, node->parent);
            if (pi >= 0) parent = (s16)pi;
        }
        AnimJoint joint = { .type = ANIM_XFORM_ROTATE, .group = (u16)i, .parent = parent };
        arrput(base->joints, joint);
    }
    base->count = (u32)arrlen(base->joints);
    return base;
}

static AnimData *gltf_build_anim_data(cgltf_data *data, cgltf_skin *skin)
{
    if (!skin || !data->animations_count) return NULL;

    AnimData *anim   = calloc(1, sizeof(AnimData));
    anim->base       = gltf_build_frame_base(skin);
    u32 joint_count  = anim->base->count;

    Transform *rest = malloc(sizeof(Transform) * joint_count);
    for (u32 ji = 0; ji < joint_count; ji++) {
        cgltf_node *node = skin->joints[ji];
        rest[ji].r = node->has_rotation    ? (Quat){node->rotation[0],    node->rotation[1],    node->rotation[2],    node->rotation[3]}    : (Quat){0,0,0,1};
        rest[ji].t = node->has_translation ? v3f(node->translation[0], node->translation[1], node->translation[2]) : v3f(0,0,0);
        rest[ji].s = node->has_scale       ? v3f(node->scale[0],       node->scale[1],       node->scale[2])       : v3f(1,1,1);
    }

    for (cgltf_size ai = 0; ai < data->animations_count; ai++) {
        cgltf_animation *canim = &data->animations[ai];

        float *times = NULL;
        for (cgltf_size ci = 0; ci < canim->channels_count; ci++) {
            u32    tc      = (u32)canim->channels[ci].sampler->input->count;
            float *ch_times = malloc(sizeof(float) * tc);
            cgltf_accessor_unpack_floats(canim->channels[ci].sampler->input, ch_times, tc);
            for (u32 ti = 0; ti < tc; ti++) {
                b32 found = false;
                for (s32 k = 0; k < arrlen(times); k++)
                    if (fabsf(times[k] - ch_times[ti]) < 0.0001f) { found = true; break; }
                if (!found) arrput(times, ch_times[ti]);
            }
            free(ch_times);
        }

        u32 time_count = (u32)arrlen(times);
        for (u32 i = 0; i < time_count - 1; i++)
            for (u32 j = i + 1; j < time_count; j++)
                if (times[j] < times[i]) { float tmp = times[i]; times[i] = times[j]; times[j] = tmp; }

        u32 frame_start = (u32)arrlen(anim->frames);

        for (u32 ti = 0; ti < time_count; ti++) {
            float      t     = times[ti];
            AnimFrame  frame = {0};
            frame.xforms = malloc(sizeof(Transform) * joint_count);
            memcpy(frame.xforms, rest, sizeof(Transform) * joint_count);

            for (cgltf_size ci = 0; ci < canim->channels_count; ci++) {
                cgltf_animation_channel *chan    = &canim->channels[ci];
                cgltf_animation_sampler *sampler = chan->sampler;
                s32 ji = gltf_find_joint(skin, chan->target_node);
                if (ji < 0) continue;

                u32 kc = (u32)sampler->input->count;
                u32 k0 = 0;
                for (u32 k = 0; k + 1 < kc; k++) {
                    float kt; cgltf_accessor_read_float(sampler->input, k + 1, &kt, 1);
                    if (kt <= t) k0 = k + 1;
                }
                u32 k1 = k0 < kc - 1 ? k0 + 1 : k0;

                float t0, t1;
                cgltf_accessor_read_float(sampler->input, k0, &t0, 1);
                cgltf_accessor_read_float(sampler->input, k1, &t1, 1);
                float alpha = (k0 != k1 && t1 > t0) ? (t - t0) / (t1 - t0) : 0.f;

                if (chan->target_path == cgltf_animation_path_type_translation) {
                    float va[3], vb[3];
                    cgltf_accessor_read_float(sampler->output, k0, va, 3);
                    cgltf_accessor_read_float(sampler->output, k1, vb, 3);
                    frame.xforms[ji].t = v3f(va[0]+alpha*(vb[0]-va[0]), va[1]+alpha*(vb[1]-va[1]), va[2]+alpha*(vb[2]-va[2]));
                } else if (chan->target_path == cgltf_animation_path_type_rotation) {
                    float va[4], vb[4];
                    cgltf_accessor_read_float(sampler->output, k0, va, 4);
                    cgltf_accessor_read_float(sampler->output, k1, vb, 4);
                    float dot  = va[0]*vb[0]+va[1]*vb[1]+va[2]*vb[2]+va[3]*vb[3];
                    float sign = dot < 0.f ? -1.f : 1.f;
                    frame.xforms[ji].r = quat_normalise((Quat){
                        va[0]+sign*alpha*(vb[0]-va[0]), va[1]+sign*alpha*(vb[1]-va[1]),
                        va[2]+sign*alpha*(vb[2]-va[2]), va[3]+sign*alpha*(vb[3]-va[3])});
                } else if (chan->target_path == cgltf_animation_path_type_scale) {
                    float va[3], vb[3];
                    cgltf_accessor_read_float(sampler->output, k0, va, 3);
                    cgltf_accessor_read_float(sampler->output, k1, vb, 3);
                    frame.xforms[ji].s = v3f(va[0]+alpha*(vb[0]-va[0]), va[1]+alpha*(vb[1]-va[1]), va[2]+alpha*(vb[2]-va[2]));
                }
            }

            arrput(anim->frames, frame);
        }

        AnimSequence seq = {0};
        const char *aname = canim->name ? canim->name : "anim";
        seq.name = (String8){ .data = (u8*)strdup(aname), .len = (u32)strlen(aname) };
        seq.loop = ANIM_LOOP;

        for (u32 fi = 0; fi < time_count; fi++) {
            u32 next   = fi + 1 < time_count ? fi + 1 : fi;
            u32 dur_ms = fi < time_count - 1
                ? (u32)((times[next] - times[fi]) * 1000.f)
                : (fi > 0 ? (u32)((times[fi] - times[fi-1]) * 1000.f) : 33u);
            AnimFrameRef ref = { .frame_idx = frame_start + fi, .duration_ms = dur_ms };
            arrput(seq.frames, ref);
        }

        arrput(anim->sequences, seq);
        arrfree(times);
    }

    free(rest);
    return anim;
}

GLTF_Model *load_gltf_model(const char *path)
{
    cgltf_options options = {0};
    cgltf_data   *data    = NULL;

    if (cgltf_parse_file(&options, path, &data) != cgltf_result_success) {
        console_write_log_alloc("Failed to parse gltf: %s", path);
        return NULL;
    }
    if (cgltf_load_buffers(&options, data, path) != cgltf_result_success) {
        console_write_log_alloc("Failed to load gltf buffers: %s", path);
        cgltf_free(data);
        return NULL;
    }

    GLTF_Model *gltf = calloc(1, sizeof(GLTF_Model));
    cgltf_skin *skin = data->skins_count > 0 ? &data->skins[0] : NULL;

    for (cgltf_size mi = 0; mi < data->meshes_count; mi++) {
        cgltf_mesh *mesh = &data->meshes[mi];
        for (cgltf_size pi = 0; pi < mesh->primitives_count; pi++) {
            Asset_Model *m = gltf_build_primitive(&mesh->primitives[pi], path, skin);
            console_write_log_alloc("-> Mesh: %zu", arrlen(m->vertices));
            if (m) { arrput(gltf->primitives, m); gltf->prim_count++; }
        }
    }

    gltf->anim_data = gltf_build_anim_data(data, skin);

    cgltf_free(data);
    log_info("Loaded gltf     %s (%u prims, %s)", path, gltf->prim_count,
             gltf->anim_data ? "animated" : "static");
    console_write_log_alloc("Loaded gltf %s (%u prims, %s)", path, gltf->prim_count,
                            gltf->anim_data ? "animated" : "static");
    return gltf;
}
