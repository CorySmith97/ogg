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


// glTF stores matrices column-major; convert to the engine's row-major format.
static Mat4 mat4_from_gltf_colmajor(const f32 *c)
{
    Mat4 m;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            m.m[i][j] = c[j * 4 + i];
    return m;
}

static s32 gltf_find_joint(cgltf_skin *skin, cgltf_node *node)
{
    for (cgltf_size i = 0; i < skin->joints_count; i++)
        if (skin->joints[i] == node) return (s32)i;
    return -1;
}

static void gltf_dir_from_path(const char *path, char *out, size_t n)
{
    strncpy(out, path, n - 1);
    out[n - 1] = '\0';
    char *slash = strrchr(out, '/');
    if (slash) *(slash + 1) = '\0';
    else out[0] = '\0';
}

GltfModel *load_model_from_gltf(const char *file)
{
    cgltf_options opts = {0};
    cgltf_data   *data = NULL;

    if (cgltf_parse_file(&opts, file, &data) != cgltf_result_success) {
        console_write_log_alloc("[gltf] parse failed: %s", file);
        return NULL;
    }
    if (cgltf_load_buffers(&opts, data, file) != cgltf_result_success) {
        console_write_log_alloc("[gltf] buffer load failed: %s", file);
        cgltf_free(data);
        return NULL;
    }

    char dir[512];
    gltf_dir_from_path(file, dir, sizeof(dir));

    GltfModel *gm   = calloc(1, sizeof(GltfModel));
    gm->mesh        = calloc(1, sizeof(Asset_Model));

    // Temp parallel skin arrays — same length as flat vertex array
    typedef struct { u16 v[4]; } J4;
    typedef struct { f32 v[4]; } W4;
    J4 *flat_j = NULL;
    W4 *flat_w = NULL;
    b32 has_skin = (data->skins_count > 0);

    // --- Geometry ---
    for (cgltf_size mi = 0; mi < data->meshes_count; mi++) {
        cgltf_mesh *mesh = &data->meshes[mi];
        for (cgltf_size pi = 0; pi < mesh->primitives_count; pi++) {
            cgltf_primitive *prim = &mesh->primitives[pi];
            if (prim->type != cgltf_primitive_type_triangles) continue;

            cgltf_accessor *pos_acc  = NULL, *norm_acc = NULL, *uv_acc = NULL;
            cgltf_accessor *j_acc    = NULL, *w_acc    = NULL;
            for (cgltf_size ai = 0; ai < prim->attributes_count; ai++) {
                cgltf_attribute *attr = &prim->attributes[ai];
                switch (attr->type) {
                    case cgltf_attribute_type_position: pos_acc  = attr->data; break;
                    case cgltf_attribute_type_normal:   norm_acc = attr->data; break;
                    case cgltf_attribute_type_texcoord: if (attr->index == 0) uv_acc = attr->data; break;
                    case cgltf_attribute_type_joints:   if (attr->index == 0) j_acc  = attr->data; break;
                    case cgltf_attribute_type_weights:  if (attr->index == 0) w_acc  = attr->data; break;
                    default: break;
                }
            }
            if (!pos_acc) continue;

            cgltf_size vc = pos_acc->count;
            f32 (*p)[3]   = malloc(vc * sizeof(*p));
            f32 (*n)[3]   = malloc(vc * sizeof(*n));
            f32 (*uv)[2]  = malloc(vc * sizeof(*uv));
            b32 have_skin = has_skin && j_acc && w_acc;
            J4  *vj       = have_skin ? malloc(vc * sizeof(J4)) : NULL;
            W4  *vw       = have_skin ? malloc(vc * sizeof(W4)) : NULL;

            for (cgltf_size vi = 0; vi < vc; vi++) {
                cgltf_accessor_read_float(pos_acc, vi, p[vi], 3);
                if (norm_acc) cgltf_accessor_read_float(norm_acc, vi, n[vi], 3);
                else          { n[vi][0] = 0; n[vi][1] = 1; n[vi][2] = 0; }
                if (uv_acc) {
                    cgltf_accessor_read_float(uv_acc, vi, uv[vi], 2);
                    uv[vi][1] = 1.0f - uv[vi][1]; // glTF V=0 is top; flip to match engine convention (V=0 = bottom)
                } else { uv[vi][0] = 0; uv[vi][1] = 0; }
                if (vj) {
                    cgltf_uint ji[4] = {0};
                    cgltf_accessor_read_uint(j_acc, vi, ji, 4);
                    for (int k = 0; k < 4; k++) vj[vi].v[k] = (u16)ji[k];
                    cgltf_accessor_read_float(w_acc, vi, vw[vi].v, 4);
                }
            }

            cgltf_size tri_count = prim->indices ? prim->indices->count / 3 : vc / 3;
            for (cgltf_size ti = 0; ti < tri_count; ti++) {
                for (int k = 0; k < 3; k++) {
                    cgltf_size idx = prim->indices
                        ? cgltf_accessor_read_index(prim->indices, ti * 3 + k)
                        : ti * 3 + k;
                    Vertex v = {
                        .position = {p[idx][0],  p[idx][1],  p[idx][2]},
                        .normal   = {n[idx][0],  n[idx][1],  n[idx][2]},
                        .uv       = {uv[idx][0], uv[idx][1], 0.0f},
                    };
                    arrput(gm->mesh->vertices, v);
                    J4 jd = vj ? vj[idx] : (J4){{0,0,0,0}};
                    W4 wd = vw ? vw[idx] : (W4){{1,0,0,0}};
                    arrput(flat_j, jd);
                    arrput(flat_w, wd);
                }
            }

            free(p); free(n); free(uv);
            if (vj) free(vj);
            if (vw) free(vw);

            // First material wins
            if (prim->material && !gm->mesh->mtl) {
                SimpleMtl *mtl = calloc(1, sizeof(SimpleMtl));
                cgltf_pbr_metallic_roughness *pbr = &prim->material->pbr_metallic_roughness;
                mtl->diffuse  = (V3f){pbr->base_color_factor[0], pbr->base_color_factor[1], pbr->base_color_factor[2]};
                f32 rough     = pbr->roughness_factor;
                mtl->ambient  = v3f_scale(mtl->diffuse, 0.1f + rough * 0.2f);
                mtl->specular = (V3f){1, 1, 1};
                mtl->specular_exponent = (1.0f - rough) * 128.0f;
                if (pbr->base_color_texture.texture) {
                    cgltf_image *img = pbr->base_color_texture.texture->image;
                    if (img && img->uri) {
                        char tex_path[512];
                        snprintf(tex_path, sizeof(tex_path), "%s%s", dir, img->uri);
                        mtl->diffuse_texture = load_texture_from_file(tex_path, true);
                    } else if (img && img->buffer_view) {
                        const u8 *raw = (const u8 *)img->buffer_view->buffer->data + img->buffer_view->offset;
                        int w, h, c;
                        stbi_set_flip_vertically_on_load(true);
                        u8 *pixels = stbi_load_from_memory(raw, (int)img->buffer_view->size, &w, &h, &c, 0);
                        if (pixels) {
                            Texture *tex  = malloc(sizeof(Texture));
                            tex->width    = w;
                            tex->height   = h;
                            tex->stride   = c;
                            tex->data     = malloc(w * h * c);
                            memcpy(tex->data, pixels, w * h * c);
                            stbi_image_free(pixels);
                            mtl->diffuse_texture = tex;
                        }
                    }
                }
                gm->mesh->mtl = mtl;
            }
        }
    }
    // Copy skin data into final flat arrays
    u32 fvc = (u32)arrlen(gm->mesh->vertices);

    // Snapshot rest pose — skinning reads from here, mesh->vertices is overwritten each frame
    if (fvc > 0) {
        gm->rest_vertices = malloc(fvc * sizeof(Vertex));
        memcpy(gm->rest_vertices, gm->mesh->vertices, fvc * sizeof(Vertex));
    }

    // Fallback material so shading works even when the glTF has no material
    if (!gm->mesh->mtl) {
        SimpleMtl *mtl        = calloc(1, sizeof(SimpleMtl));
        mtl->diffuse          = (V3f){0.8f, 0.8f, 0.8f};
        mtl->ambient          = (V3f){0.15f, 0.15f, 0.15f};
        mtl->specular         = (V3f){0.5f, 0.5f, 0.5f};
        mtl->specular_exponent = 32.0f;
        gm->mesh->mtl         = mtl;
    }

    gm->flat_vertex_count = fvc;
    if (has_skin && flat_j) {
        gm->vert_joints  = malloc(fvc * sizeof(*gm->vert_joints));
        gm->vert_weights = malloc(fvc * sizeof(*gm->vert_weights));
        for (u32 i = 0; i < fvc; i++) {
            memcpy(gm->vert_joints[i],  flat_j[i].v, 4 * sizeof(u16));
            memcpy(gm->vert_weights[i], flat_w[i].v, 4 * sizeof(f32));
        }
    }
    arrfree(flat_j);
    arrfree(flat_w);

    // Build base_positions / index_buffer / skin_groups (same as OBJ loader)
    {
        gm->mesh->index_buffer = malloc(sizeof(u32) * fvc);
        for (u32 j = 0; j < fvc; j++) {
            V3f pos   = gm->mesh->vertices[j].position;
            u32 found = UINT32_MAX;
            for (u32 k = 0; k < (u32)arrlen(gm->mesh->base_positions); k++) {
                if (v3f_equal(gm->mesh->base_positions[k], pos)) { found = k; break; }
            }
            if (found == UINT32_MAX) {
                found = (u32)arrlen(gm->mesh->base_positions);
                arrput(gm->mesh->base_positions, pos);
            }
            gm->mesh->index_buffer[j] = found;
        }
        gm->mesh->base_count  = (u32)arrlen(gm->mesh->base_positions);
        gm->mesh->skin_groups = malloc(sizeof(u16) * gm->mesh->base_count);
        for (u32 i = 0; i < gm->mesh->base_count; i++)
            gm->mesh->skin_groups[i] = ANIM_GROUP_STATIC;
    }

    // --- Joints ---
    if (data->skins_count > 0) {
        cgltf_skin *skin    = &data->skins[0];
        gm->joint_count     = (u32)skin->joints_count;
        gm->joints          = calloc(gm->joint_count, sizeof(GltfJoint));
        for (u32 ji = 0; ji < gm->joint_count; ji++) {
            cgltf_node *node = skin->joints[ji];
            GltfJoint  *j    = &gm->joints[ji];
            j->parent  = (s16)(node->parent ? gltf_find_joint(skin, node->parent) : -1);
            j->rest_t  = node->has_translation
                ? (V3f){node->translation[0], node->translation[1], node->translation[2]}
                : (V3f){0, 0, 0};
            j->rest_r  = node->has_rotation
                ? (Quat){node->rotation[0], node->rotation[1], node->rotation[2], node->rotation[3]}
                : quat_identity();
            j->rest_s  = node->has_scale
                ? (V3f){node->scale[0], node->scale[1], node->scale[2]}
                : (V3f){1, 1, 1};
            if (skin->inverse_bind_matrices) {
                f32 raw[16];
                cgltf_accessor_read_float(skin->inverse_bind_matrices, ji, raw, 16);
                j->inv_bind = mat4_from_gltf_colmajor(raw);
            } else {
                j->inv_bind = mat4_identity();
            }
        }
    }

    // --- Animations ---
    for (cgltf_size ai = 0; ai < data->animations_count; ai++) {
        cgltf_animation *anim = &data->animations[ai];
        GltfAnim ga = {0};
        snprintf(ga.name, sizeof(ga.name), "%s", anim->name ? anim->name : "");

        for (cgltf_size ci = 0; ci < anim->channels_count; ci++) {
            cgltf_animation_channel *ch = &anim->channels[ci];
            if (!ch->target_node || !data->skins_count) continue;
            s32 jidx = gltf_find_joint(&data->skins[0], ch->target_node);
            if (jidx < 0) continue;

            GltfChannel gc = {.joint_idx = (u32)jidx};
            switch (ch->target_path) {
                case cgltf_animation_path_type_translation: gc.type = GLTF_CHAN_TRANSLATION; break;
                case cgltf_animation_path_type_rotation:    gc.type = GLTF_CHAN_ROTATION;    break;
                case cgltf_animation_path_type_scale:       gc.type = GLTF_CHAN_SCALE;       break;
                default: continue;
            }
            switch (ch->sampler->interpolation) {
                case cgltf_interpolation_type_step:         gc.interp = GLTF_INTERP_STEP;        break;
                case cgltf_interpolation_type_cubic_spline: gc.interp = GLTF_INTERP_CUBICSPLINE; break;
                default:                                    gc.interp = GLTF_INTERP_LINEAR;      break;
            }

            gc.count      = (u32)ch->sampler->input->count;
            gc.times      = malloc(gc.count * sizeof(f32));
            u32 comps     = (gc.type == GLTF_CHAN_ROTATION) ? 4 : 3;
            gc.values     = malloc(gc.count * comps * sizeof(f32));
            for (u32 ti = 0; ti < gc.count; ti++) {
                cgltf_accessor_read_float(ch->sampler->input,  ti, &gc.times[ti],           1);
                cgltf_accessor_read_float(ch->sampler->output, ti, &gc.values[ti * comps], comps);
            }
            if (gc.count > 0 && gc.times[gc.count - 1] > ga.duration)
                ga.duration = gc.times[gc.count - 1];
            arrput(ga.channels, gc);
        }
        ga.channel_count = (u32)arrlen(ga.channels);
        arrput(gm->anims, ga);
    }
    gm->anim_count = (u32)arrlen(gm->anims);

    console_write_log_alloc("[gltf] %s | verts: %u  joints: %u  anims: %u",
        file, fvc, gm->joint_count, gm->anim_count);
    cgltf_free(data);
    return gm;
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
