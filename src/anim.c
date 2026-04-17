// Concatenate a parent and child Transform in local space.
// The child's translation is rotated into the parent's frame,
// then added to the parent's translation.
static Transform transform_concat(Transform parent, Transform child)
{
    Transform out;
    out.r = quat_mul(parent.r, child.r);
    out.t = v3f_add(parent.t, quat_rotate_v3f(parent.r, child.t));
    out.s = v3f(parent.s.x * child.s.x,
                parent.s.y * child.s.y,
                parent.s.z * child.s.z);
    return out;
}

// Apply a Transform to a single rest-pose position.
// Order: scale first (in local space), then rotate, then translate.
static V3f transform_apply(Transform xf, V3f v)
{
    v = v3f(v.x * xf.s.x, v.y * xf.s.y, v.z * xf.s.z);
    v = quat_rotate_v3f(xf.r, v);
    v = v3f_add(v, xf.t);
    return v;
}

void anim_state_init(AnimState *state, AnimData *data, Asset_Model *model)
{
    state->data       = data;
    state->seq_idx    = 0;
    state->frame_idx  = 0;
    state->time_acc_ms = 0.0f;
    state->animated_positions = malloc(sizeof(V3f) * model->base_count);
}

void anim_state_play(AnimState *state, const char *name)
{
    AnimData *data = state->data;
    for (u32 i = 0; i < (u32)arrlen(data->sequences); i++) {
        // String8 comparison
        AnimSequence *seq = &data->sequences[i];
        if (seq->name.len == strlen(name) &&
            memcmp(seq->name.data, name, seq->name.len) == 0)
        {
            state->seq_idx     = i;
            state->frame_idx   = 0;
            state->time_acc_ms = 0.0f;
            return;
        }
    }
}

void anim_update(AnimState *state, f32 dt_ms)
{
    AnimData     *data = state->data;
    if (!data || arrlen(data->sequences) == 0) return;

    AnimSequence *seq  = &data->sequences[state->seq_idx];
    if (arrlen(seq->frames) == 0) return;

    state->time_acc_ms += dt_ms;

    AnimFrameRef *ref = &seq->frames[state->frame_idx];
    while (state->time_acc_ms >= (f32)ref->duration_ms) {
        state->time_acc_ms -= (f32)ref->duration_ms;
        state->frame_idx++;

        if (state->frame_idx >= (u32)arrlen(seq->frames)) {
            switch (seq->loop) {
                case ANIM_LOOP: state->frame_idx = 0; break;
                case ANIM_HOLD: state->frame_idx = (u32)arrlen(seq->frames) - 1; break;
                case ANIM_STOP: state->frame_idx = 0; state->time_acc_ms = 0.0f; return;
            }
        }

        ref = &seq->frames[state->frame_idx];
    }
}

// Core deformation pass.
// Computes world transforms top-down through the joint hierarchy, then
// applies each group's world transform to every base position in that group,
// then scatters results back into the flat vertex array the renderer reads.
void anim_apply_lerp(AnimState *state, Asset_Model *model,
                     AnimFrame *fa, AnimFrame *fb, f32 t)
{
    FrameBase *base  = state->data->base;
    u32        jcount = base->count;

    // Step 1 — compute world transforms.
    // Joints are stored in parent-before-child order, so we can do a single
    // forward pass: each joint's world transform is its local transform
    // concatenated onto its already-computed parent world transform.
    Transform *world = alloca(sizeof(Transform) * jcount);
    for (u32 j = 0; j < jcount; j++) {
        AnimJoint *joint = &base->joints[j];

        // Lerp between the two frames for this joint
        Transform la = fa->xforms[j];
        Transform lb = fb->xforms[j];
        Transform local;
        local.t = v3f_add(v3f_scale(la.t, 1.0f - t), v3f_scale(lb.t, t));
        local.s = v3f_add(v3f_scale(la.s, 1.0f - t), v3f_scale(lb.s, t));
        // Shortest-path quaternion lerp (nlerp)
        f32 dot = la.r.x*lb.r.x + la.r.y*lb.r.y + la.r.z*lb.r.z + la.r.w*lb.r.w;
        f32 sign = (dot < 0.0f) ? -1.0f : 1.0f;
        local.r = quat_normalise((Quat){
            la.r.x + sign * t * (lb.r.x - la.r.x),
            la.r.y + sign * t * (lb.r.y - la.r.y),
            la.r.z + sign * t * (lb.r.z - la.r.z),
            la.r.w + sign * t * (lb.r.w - la.r.w),
        });

        if (joint->parent == -1) {
            world[j] = local;
        } else {
            world[j] = transform_concat(world[joint->parent], local);
        }
    }

    // Step 2 — deform base positions.
    V3f *out = state->animated_positions;
    for (u32 i = 0; i < model->base_count; i++) {
        u16 g = model->skin_groups[i];
        if (g == ANIM_GROUP_STATIC || g >= jcount) {
            out[i] = model->base_positions[i];
        } else {
            out[i] = transform_apply(world[g], model->base_positions[i]);
        }
    }

    // Step 3 — scatter animated positions back into the flat vertex array.
    u32 flat_count = (u32)arrlen(model->vertices);
    for (u32 j = 0; j < flat_count; j++) {
        model->vertices[j].position = out[model->index_buffer[j]];
    }
}

void anim_apply(AnimState *state, Asset_Model *model)
{
    AnimData     *data = state->data;
    if (!data || arrlen(data->frames) == 0) return;

    AnimSequence *seq  = &data->sequences[state->seq_idx];
    if (arrlen(seq->frames) == 0) return;

    u32 cur  = seq->frames[state->frame_idx].frame_idx;
    u32 next_frame_idx = (state->frame_idx + 1) % (u32)arrlen(seq->frames);
    u32 next = seq->frames[next_frame_idx].frame_idx;

    f32 dur  = (f32)seq->frames[state->frame_idx].duration_ms;
    f32 t    = (dur > 0.0f) ? (state->time_acc_ms / dur) : 0.0f;

    anim_apply_lerp(state, model,
                    &data->frames[cur],
                    &data->frames[next],
                    t);
}

// AI-GENERATED: pose-apply helper for the model editor.
// Mirrors anim_apply_lerp but takes a pose array directly (no AnimState, no lerp).
// pose[j] must be indexed such that pose[joint.group] drives the joint — this
// matches the flat "group == joint index" FrameBase that anim_editor_build_skel creates.
// Review: if you add hierarchical joints, verify the parent-concat order is still correct.
void anim_apply_pose(FrameBase *base, Transform *pose, Asset_Model *model)
{
    u32 jcount = base->count;
    if (jcount == 0) return;

    // Step 1: compute world transforms top-down (parents are stored before children)
    Transform *world = alloca(sizeof(Transform) * jcount);
    for (u32 j = 0; j < jcount; j++) {
        AnimJoint *joint = &base->joints[j];
        if (joint->parent == -1)
            world[j] = pose[j];
        else
            world[j] = transform_concat(world[joint->parent], pose[j]);
    }

    // Step 2: deform base positions
    V3f *out = malloc(sizeof(V3f) * model->base_count);
    for (u32 i = 0; i < model->base_count; i++) {
        u16 g = model->skin_groups[i];
        if (g == ANIM_GROUP_STATIC || g >= jcount)
            out[i] = model->base_positions[i];
        else
            out[i] = transform_apply(world[g], model->base_positions[i]);
    }

    // Step 3: scatter back into flat vertex array
    u32 flat = (u32)arrlen(model->vertices);
    for (u32 j = 0; j < flat; j++)
        model->vertices[j].position = out[model->index_buffer[j]];

    free(out);
}

// ---- file loaders -----------------------------------------------------------

FrameBase *skel_load(const char *file)
{
    FILE *f = fopen(file, "r");
    if (!f) return NULL;

    FrameBase *base = calloc(1, sizeof(FrameBase));
    char *line = NULL;
    size_t len;
    u32 joint_count = 0;

    if (getline(&line, &len, f) > 0)
        sscanf(line, "%u", &joint_count);

    for (u32 j = 0; j < joint_count; j++) {
        if (getline(&line, &len, f) < 0) break;
        AnimJoint joint = {0};
        char type_str[16];
        sscanf(line, "%15s %hu %hd", type_str, &joint.group, &joint.parent);
        if      (strcmp(type_str, "translate") == 0) joint.type = ANIM_XFORM_TRANSLATE;
        else if (strcmp(type_str, "scale")     == 0) joint.type = ANIM_XFORM_SCALE;
        else                                         joint.type = ANIM_XFORM_ROTATE;
        arrput(base->joints, joint);
    }
    base->count = (u32)arrlen(base->joints);

    free(line);
    fclose(f);
    return base;
}

// .anim file format:
//   frame
//     <rx ry rz rw>  <tx ty tz>  <sx sy sz>    <- one line per joint
//   end
//   sequence <name> <loop|hold|stop>
//     <frame_idx> <duration_ms>
//     ...
//   end
AnimData *anim_data_load(const char *skel_file, const char *anim_file)
{
    AnimData *data = calloc(1, sizeof(AnimData));
    data->base = skel_load(skel_file);
    if (!data->base) {
        data->base = calloc(1, sizeof(FrameBase));
    }
    u32 joint_count = data->base->count;

    FILE *f = fopen(anim_file, "r");
    if (!f) return data;

    typedef enum { PARSE_IDLE, PARSE_FRAME, PARSE_SEQ } ParseState;
    ParseState    state     = PARSE_IDLE;
    AnimFrame     cur_frame = {0};
    AnimSequence  cur_seq   = {0};
    u32           joint_idx = 0;

    char *line = NULL;
    size_t len;

    while (getline(&line, &len, f) != -1) {
        line[strcspn(line, "\n\r")] = '\0';
        if (line[0] == '#' || line[0] == '\0') continue;

        if (strcmp(line, "frame") == 0) {
            state           = PARSE_FRAME;
            cur_frame.xforms = malloc(sizeof(Transform) * (joint_count ? joint_count : 1));
            joint_idx       = 0;

        } else if (strcmp(line, "end") == 0) {
            if (state == PARSE_FRAME) {
                arrput(data->frames, cur_frame);
            } else if (state == PARSE_SEQ) {
                arrput(data->sequences, cur_seq);
            }
            state = PARSE_IDLE;

        } else if (strncmp(line, "sequence", 8) == 0) {
            state        = PARSE_SEQ;
            cur_seq      = (AnimSequence){0};
            char name_buf[64] = {0};
            char loop_str[16] = {0};
            sscanf(line, "sequence %63s %15s", name_buf, loop_str);
            cur_seq.name = (String8){ .data = (u8*)strdup(name_buf),
                                      .len  = (u32)strlen(name_buf) };
            if      (strcmp(loop_str, "hold") == 0) cur_seq.loop = ANIM_HOLD;
            else if (strcmp(loop_str, "stop") == 0) cur_seq.loop = ANIM_STOP;
            else                                    cur_seq.loop = ANIM_LOOP;

        } else if (state == PARSE_FRAME && joint_idx < joint_count) {
            Transform *xf = &cur_frame.xforms[joint_idx];
            sscanf(line, "%f %f %f %f  %f %f %f  %f %f %f",
                   &xf->r.x, &xf->r.y, &xf->r.z, &xf->r.w,
                   &xf->t.x, &xf->t.y, &xf->t.z,
                   &xf->s.x, &xf->s.y, &xf->s.z);
            joint_idx++;

        } else if (state == PARSE_SEQ) {
            AnimFrameRef ref = {0};
            if (sscanf(line, "%u %u", &ref.frame_idx, &ref.duration_ms) == 2)
                arrput(cur_seq.frames, ref);
        }
    }

    free(line);
    fclose(f);
    return data;
}
