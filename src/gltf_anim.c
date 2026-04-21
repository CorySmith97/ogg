// Build a TRS matrix in the engine's row-major convention.
// Equivalent to T * R * S: scale first, then rotate, then translate.
static Mat4 gltf_mat4_trs(V3f t, Quat r, V3f s)
{
    Mat4 m = mat4_from_quat(r);
    // Scale columns 0,1,2 by sx,sy,sz
    for (int row = 0; row < 3; row++) {
        m.m[row][0] *= s.x;
        m.m[row][1] *= s.y;
        m.m[row][2] *= s.z;
    }
    // Translation in column 3
    m.m[0][3] = t.x;
    m.m[1][3] = t.y;
    m.m[2][3] = t.z;
    m.m[3][0] = 0.0f; m.m[3][1] = 0.0f; m.m[3][2] = 0.0f; m.m[3][3] = 1.0f;
    return m;
}

static Mat4 gltf_build_global_joint(GltfModel *gm, V3f *jt, Quat *jr, V3f *js,
                                    Mat4 *global, b32 *built, u32 ji)
{
    if (built[ji]) return global[ji];

    Mat4 local = gltf_mat4_trs(jt[ji], jr[ji], js[ji]);
    s16 par = gm->joints[ji].parent;

    if (par < 0) {
        global[ji] = local;
    } else {
        Mat4 parent_global = gltf_build_global_joint(gm, jt, jr, js, global, built, (u32)par);
        global[ji] = mat4_mul(parent_global, local);
    }

    built[ji] = true;
    return global[ji];
}

static Quat gltf_quat_slerp(Quat a, Quat b, f32 t)
{
    f32 dot = a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
    if (dot < 0.0f) { b.x=-b.x; b.y=-b.y; b.z=-b.z; b.w=-b.w; dot=-dot; }
    if (dot > 0.9995f) {
        return quat_normalise((Quat){
            a.x + t*(b.x-a.x), a.y + t*(b.y-a.y),
            a.z + t*(b.z-a.z), a.w + t*(b.w-a.w),
        });
    }
    f32 theta_0    = acosf(dot);
    f32 theta      = theta_0 * t;
    f32 sin_t      = sinf(theta);
    f32 sin_t0     = sinf(theta_0);
    f32 s0         = cosf(theta) - dot * sin_t / sin_t0;
    f32 s1         = sin_t / sin_t0;
    return (Quat){ s0*a.x+s1*b.x, s0*a.y+s1*b.y, s0*a.z+s1*b.z, s0*a.w+s1*b.w };
}

static V3f gltf_sample_vec3(GltfChannel *ch, f32 time)
{
    if (ch->count == 0) return (V3f){0,0,0};
    if (ch->count == 1 || time <= ch->times[0])
        return (V3f){ch->values[0], ch->values[1], ch->values[2]};
    u32 last = ch->count - 1;
    if (time >= ch->times[last])
        return (V3f){ch->values[last*3+0], ch->values[last*3+1], ch->values[last*3+2]};
    u32 i = 0;
    while (i < last && ch->times[i+1] <= time) i++;
    f32 alpha = (ch->times[i+1] > ch->times[i])
        ? (time - ch->times[i]) / (ch->times[i+1] - ch->times[i]) : 0.0f;
    V3f a = {ch->values[i*3+0],     ch->values[i*3+1],     ch->values[i*3+2]};
    V3f b = {ch->values[(i+1)*3+0], ch->values[(i+1)*3+1], ch->values[(i+1)*3+2]};
    return (V3f){a.x+alpha*(b.x-a.x), a.y+alpha*(b.y-a.y), a.z+alpha*(b.z-a.z)};
}

static Quat gltf_sample_quat(GltfChannel *ch, f32 time)
{
    if (ch->count == 0) return quat_identity();
    if (ch->count == 1 || time <= ch->times[0])
        return (Quat){ch->values[0], ch->values[1], ch->values[2], ch->values[3]};
    u32 last = ch->count - 1;
    if (time >= ch->times[last])
        return (Quat){ch->values[last*4+0], ch->values[last*4+1], ch->values[last*4+2], ch->values[last*4+3]};
    u32 i = 0;
    while (i < last && ch->times[i+1] <= time) i++;
    f32 alpha = (ch->times[i+1] > ch->times[i])
        ? (time - ch->times[i]) / (ch->times[i+1] - ch->times[i]) : 0.0f;
    Quat a = {ch->values[i*4+0],     ch->values[i*4+1],     ch->values[i*4+2],     ch->values[i*4+3]};
    Quat b = {ch->values[(i+1)*4+0], ch->values[(i+1)*4+1], ch->values[(i+1)*4+2], ch->values[(i+1)*4+3]};
    return gltf_quat_slerp(a, b, alpha);
}

void gltf_anim_init(GltfAnimState *state, GltfModel *model, const char *anim_name)
{
    state->model    = model;
    state->anim_idx = 0;
    state->time     = 0.0f;
    state->loop     = true;
    if (anim_name)
        gltf_anim_play(state, anim_name);
}

void gltf_anim_play(GltfAnimState *state, const char *anim_name)
{
    GltfModel *gm = state->model;
    for (u32 i = 0; i < gm->anim_count; i++) {
        if (strcmp(gm->anims[i].name, anim_name) == 0) {
            state->anim_idx = i;
            state->time     = 0.0f;
            return;
        }
    }
    console_write_log_alloc("[gltf_anim] animation not found: %s", anim_name);
}

void gltf_anim_update(GltfAnimState *state, f32 dt)
{
    if (!state->model || state->model->anim_count == 0) return;
    GltfAnim *anim = &state->model->anims[state->anim_idx];
    state->time += dt;
    if (state->loop && anim->duration > 0.0f)
        while (state->time > anim->duration) state->time -= anim->duration;
}

void gltf_anim_apply(GltfAnimState *state)
{
    GltfModel *gm = state->model;
    if (!gm || !gm->rest_vertices || !gm->vert_joints || gm->joint_count == 0) return;

    GltfAnim *anim = &gm->anims[state->anim_idx];
    u32       jc   = gm->joint_count;

    // Start every joint at its rest pose
    V3f  jt[jc]; Quat jr[jc]; V3f js[jc];
    for (u32 ji = 0; ji < jc; ji++) {
        jt[ji] = gm->joints[ji].rest_t;
        jr[ji] = gm->joints[ji].rest_r;
        js[ji] = gm->joints[ji].rest_s;
    }

    // Override with sampled animation channels
    for (u32 ci = 0; ci < anim->channel_count; ci++) {
        GltfChannel *ch = &anim->channels[ci];
        u32 ji = ch->joint_idx;
        if (ji >= jc) continue;
        switch (ch->type) {
            case GLTF_CHAN_TRANSLATION: jt[ji] = gltf_sample_vec3(ch, state->time); break;
            case GLTF_CHAN_ROTATION:    jr[ji] = gltf_sample_quat(ch, state->time); break;
            case GLTF_CHAN_SCALE:       js[ji] = gltf_sample_vec3(ch, state->time); break;
        }
    }

    // 1. build global joint transforms
    Mat4 global[jc];
    // Linear blend skinning
    u32 fvc = gm->flat_vertex_count;

    // 2. build final skinning matrices
    Mat4 jmat[jc];
    for (u32 ji = 0; ji < jc; ji++) {
        jmat[ji] = mat4_mul(global[ji], gm->joints[ji].inv_bind);
    }

    // 3. skin vertices
    for (u32 vi = 0; vi < fvc; vi++) {
        for (int k = 0; k < 4; k++) {
            u32 ji = gm->vert_joints[vi][k];
            Mat4 *m = &jmat[ji];
        }
    }
    for (u32 vi = 0; vi < fvc; vi++) {
        V3f rp = gm->rest_vertices[vi].position;
        V3f rn = gm->rest_vertices[vi].normal;
        V3f pos = {0,0,0}, nor = {0,0,0};

        for (int k = 0; k < 4; k++) {
            f32 w = gm->vert_weights[vi][k];
            if (w == 0.0f) continue;
            u32 ji = gm->vert_joints[vi][k];
            if (ji >= jc) continue;
            Mat4 *m = &jmat[ji];

            V3f tp = v3f_translate_by_mat4(rp, *m);
            pos.x += w * tp.x; pos.y += w * tp.y; pos.z += w * tp.z;

            // Rotate normal by upper-left 3x3 (no translation, no inv-transpose for now)
            nor.x += w * (rn.x*m->m[0][0] + rn.y*m->m[0][1] + rn.z*m->m[0][2]);
            nor.y += w * (rn.x*m->m[1][0] + rn.y*m->m[1][1] + rn.z*m->m[1][2]);
            nor.z += w * (rn.x*m->m[2][0] + rn.y*m->m[2][1] + rn.z*m->m[2][2]);
        }

        gm->mesh->vertices[vi].position = pos;
        gm->mesh->vertices[vi].normal   = v3f_normalize(nor);
    }
}
