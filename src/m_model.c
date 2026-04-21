void model_editor_camera_update(void);

// ---- helpers ---------------------------------------------------------------

// Transform to view space — used for rendering (renderer expects view-space verts).
static V3f model_to_view(V3f p, Mat3 rot, V3f world_pos)
{
    p = v3f_mul_mat3(p, rot);
    p.z = -p.z;
    p = v3f_add(p, world_pos);
    return v3f_translate_by_mat4(p, camera_matrix(m_editor.camera));
}

// Transform to world space — used for ray-triangle picking.
// get_mouse_ray returns a world-space ray, so the triangles must be in the
// same space. Applying the camera matrix here would put them in clip space
// and make Möller-Trumbore produce no hits.
static V3f model_to_world(V3f p, Mat3 rot, V3f world_pos)
{
    p = v3f_mul_mat3(p, rot);
    p.z = -p.z;
    p = v3f_add(p, world_pos);
    return p;
}

// Draw every triangle tinted by the group color of its first vertex.
// Triangles with ANIM_GROUP_STATIC get a neutral grey so unpainted geometry
// is visually distinct from group-0 geometry.
static void draw_skin_groups(Asset_Model *model, Mat3 rot)
{
    UNUSED(rot);
    Mat4 view = camera_matrix(m_editor.camera);
    Color grey = {160, 160, 160, 255};

    for (s32 i = 0; i < arrlen(model->vertices); i += 3) {
        V3f p1 = v3f_translate_by_mat4(m_editor.world_positions[i    ], view);
        V3f p2 = v3f_translate_by_mat4(m_editor.world_positions[i + 1], view);
        V3f p3 = v3f_translate_by_mat4(m_editor.world_positions[i + 2], view);

        if (p1.z < NEAR || p2.z < NEAR || p3.z < NEAR) continue;

        // All three vertices of a painted triangle share the same group, so
        // sampling vertex 0 of the triangle is enough to get the group color.
        u16 g = model->skin_groups[model->index_buffer[i]];
        Color c = (g == ANIM_GROUP_STATIC) ? grey : GROUP_PALETTE[g % 16];

        Color colors[3] = {c, c, c};
        V3f   uvs[3]    = {0};
        renderer_push_triangle(p1, p2, p3, colors, uvs, NULL, TRIANGLE_WRITE_OVER_Z);
    }
}

// Draw the hovered triangle as a wireframe overlay so the user can see
// exactly what they are about to paint before clicking.
static void draw_hovered_tri(Asset_Model *model, Mat3 rot, s32 tri_flat_idx)
{
    UNUSED(rot); UNUSED(model);
    if (tri_flat_idx < 0) return;

    Mat4 view = camera_matrix(m_editor.camera);
    V3f p1 = v3f_translate_by_mat4(m_editor.world_positions[tri_flat_idx    ], view);
    V3f p2 = v3f_translate_by_mat4(m_editor.world_positions[tri_flat_idx + 1], view);
    V3f p3 = v3f_translate_by_mat4(m_editor.world_positions[tri_flat_idx + 2], view);

    if (p1.z < NEAR || p2.z < NEAR || p3.z < NEAR) return;

    Color white[3]  = {{255, 255, 255, 255}, {255, 255, 255, 255}, {255, 255, 255, 255}};
    V3f   uvs[3]    = {0};
    renderer_push_triangle(p1, p2, p3, white, uvs, NULL,
                           TRIANGLE_WIRE_FRAME | TRIANGLE_WRITE_OVER_Z);
}

// Find the closest triangle under the mouse ray.
// Returns the flat vertex index of the first vertex of that triangle, or -1.
static s32 pick_triangle(Asset_Model *model, Ray ray)
{
    s32 best     = -1;
    f32 best_dist = FLT_MAX;

    for (s32 i = 0; i < arrlen(model->vertices); i += 3) {
        RayCollision col = get_ray_collision_triangle(
            ray,
            m_editor.world_positions[i    ],
            m_editor.world_positions[i + 1],
            m_editor.world_positions[i + 2]);
        if (col.hit && col.distance < best_dist) {
            best_dist = col.distance;
            best      = i;
        }
    }
    return best;
}

// Assign the active group to all three base positions of a flat triangle.
static void paint_triangle(Asset_Model *model, s32 tri_flat_idx, u16 group)
{
    if (tri_flat_idx < 0) return;
    for (s32 v = 0; v < 3; v++) {
        u32 base_idx = model->index_buffer[tri_flat_idx + v];
        model->skin_groups[base_idx] = group;
    }
}

// BFS flood-fill from a triangle, painting all connected triangles that share
// the same skin group as the start triangle.
static void flood_fill_paint(Asset_Model *model, s32 start_tri, u16 group)
{
    if (start_tri < 0) return;
    s32 tri_count = (s32)arrlen(model->vertices) / 3;
    u16 target_group = model->skin_groups[model->index_buffer[start_tri]];

    b32 *visited = calloc(tri_count, sizeof(b32));
    s32 *queue   = malloc(tri_count * sizeof(s32));
    s32 head = 0, tail = 0;

    visited[start_tri / 3] = true;
    queue[tail++] = start_tri;

    while (head < tail) {
        s32 tri = queue[head++];
        paint_triangle(model, tri, group);

        u32 b0 = model->index_buffer[tri    ];
        u32 b1 = model->index_buffer[tri + 1];
        u32 b2 = model->index_buffer[tri + 2];

        for (s32 i = 0; i < tri_count * 3; i += 3) {
            s32 idx = i / 3;
            if (visited[idx]) continue;
            u32 t0 = model->index_buffer[i    ];
            u32 t1 = model->index_buffer[i + 1];
            u32 t2 = model->index_buffer[i + 2];
            b32 shares = (t0==b0||t0==b1||t0==b2 ||
                          t1==b0||t1==b1||t1==b2 ||
                          t2==b0||t2==b1||t2==b2);
            if (shares && model->skin_groups[t0] == target_group) {
                visited[idx] = true;
                queue[tail++] = i;
            }
        }
    }

    free(visited);
    free(queue);
}

// Convert a world-space point to integer screen coordinates.
// Returns (-1,-1) if behind the camera.
static V2i world_to_screen(V3f world)
{
    Mat4 view = camera_matrix(m_editor.camera);
    V3f  vs   = v3f_translate_by_mat4(world, view);
    if (vs.z < NEAR) return v2i(-1, -1);
    return to_screen(project(vs));
}

// Paint all triangles whose screen-space centroid falls within [x0,x1] x [y0,y1].
static void box_paint_region(Asset_Model *model, V2f a, V2f b, u16 group)
{
    f32 x0 = fminf(a.x, b.x), x1 = fmaxf(a.x, b.x);
    f32 y0 = fminf(a.y, b.y), y1 = fmaxf(a.y, b.y);

    for (s32 i = 0; i < arrlen(model->vertices); i += 3) {
        V3f wc = v3f_scale(
            v3f_add(v3f_add(m_editor.world_positions[i],
                            m_editor.world_positions[i+1]),
                            m_editor.world_positions[i+2]),
            1.0f / 3.0f);
        V2i sc = world_to_screen(wc);
        if (sc.x < 0) continue;
        if (sc.x >= x0 && sc.x <= x1 && sc.y >= y0 && sc.y <= y1)
            paint_triangle(model, i, group);
    }
}

// ---- Nuklear panel ---------------------------------------------------------

static void draw_skin_panel(void)
{
    struct nk_context *ui = platform_ctx.ui;
    static const nk_flags flags =
        NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_TITLE;

    if (nk_begin(ui, "Skin Editor",
                 nk_rect(GAME_WIDTH - 220, 0, 400, GAME_HEIGHT / 2), flags))
    {
        // Mode toggle
        nk_layout_row_dynamic(ui, 24, 2);
        if (nk_button_label(ui, "Select"))
            m_editor.mode = MODEL_EDITOR_MODE_SELECT;
        if (nk_button_label(ui, "Paint"))
            m_editor.mode = MODEL_EDITOR_MODE_PAINT;

        // Current mode label
        nk_layout_row_dynamic(ui, 18, 1);
        nk_label_wrap(ui, m_editor.mode == MODEL_EDITOR_MODE_PAINT
                         ? "Mode: PAINT  [G to toggle]"
                         : "Mode: SELECT [G to toggle]");

        // Group selector
        nk_layout_row_dynamic(ui, 18, 1);
        nk_label_wrap(ui, "Active group:");

        char label[8];
        nk_layout_row_dynamic(ui, 28, 4);
        for (u16 g = 0; g < 16; g++) {
            snprintf(label, sizeof(label), "%s%d", (g == m_editor.active_group) ? ">" : " ", g);
            if (nk_button_label(ui, label))
                m_editor.active_group = g;
        }

        // Stats
        if (m_editor.selected) {
            nk_layout_row_dynamic(ui, 18, 1);
            char info[64];
            snprintf(info, sizeof(info), "Base verts: %u", m_editor.selected->base_count);
            nk_label_wrap(ui, info);

            u32 painted = 0;
            for (u32 i = 0; i < m_editor.selected->base_count; i++) {
                if (m_editor.selected->skin_groups[i] == m_editor.active_group)
                    painted++;
            }
            snprintf(info, sizeof(info), "In group %d: %u", m_editor.active_group, painted);
            nk_label_wrap(ui, info);
        }

        // Flood fill
        nk_layout_row_dynamic(ui, 18, 1);
        nk_label_wrap(ui, "F = flood fill from hover");
        nk_layout_row_dynamic(ui, 24, 1);
        if (nk_button_label(ui, "Flood Fill") &&
            m_editor.mode == MODEL_EDITOR_MODE_PAINT &&
            m_editor.hovered_tri >= 0)
        {
            flood_fill_paint(m_editor.selected, m_editor.hovered_tri, m_editor.active_group);
        }

        // Save skin
        nk_layout_row_dynamic(ui, 24, 1);
        if (nk_button_label(ui, "Save Skin")) {
            char path[300];
            snprintf(path, sizeof(path), "data/%s.skin", m_editor.model_name);
            skin_save(path, m_editor.selected);
        }
    }
    nk_end(ui);
}

// Shortest-arc quaternion rotating unit vector a onto unit vector b.
static Quat quat_from_to(V3f a, V3f b)
{
    V3f axis = v3f_cross(a, b);
    f32 dot  = v3f_dot(a, b);
    if (dot < -0.9999f) {
        V3f ref = (fabsf(a.x) < 0.9f) ? v3f(1,0,0) : v3f(0,1,0);
        axis = v3f_normalize(v3f_cross(a, ref));
        return (Quat){axis.x, axis.y, axis.z, 0.0f};
    }
    f32 s = sqrtf((1.0f + dot) * 2.0f);
    f32 inv = 1.0f / s;
    return quat_normalise((Quat){axis.x*inv, axis.y*inv, axis.z*inv, s*0.5f});
}

// Draw a diamond-shaped bone between two world-space points.
// Both triangles share the wide end; the tip points toward b.
static void draw_bone_line(V3f a, V3f b, Color color)
{
    V3f d = v3f_sub(b, a);
    f32 len = sqrtf(v3f_dot(d, d));
    if (len < 0.0001f) return;
    V3f dir = v3f_scale(d, 1.0f / len);

    V3f ref  = (fabsf(dir.y) < 0.9f) ? v3f(0,1,0) : v3f(1,0,0);
    V3f perp = v3f_normalize(v3f_cross(dir, ref));
    f32 s    = len * 0.10f;
    // Knob is 20% of the way from a toward b
    V3f knob = v3f_add(a, v3f_scale(dir, len * 0.2f));
    V3f mp   = v3f_add(knob, v3f_scale(perp, s));
    V3f mm   = v3f_sub(knob, v3f_scale(perp, s));

    Mat4 view = camera_matrix(renderer.camera);
    V3f va  = v3f_translate_by_mat4(a,    view);
    V3f vb  = v3f_translate_by_mat4(b,    view);
    V3f vmp = v3f_translate_by_mat4(mp,   view);
    V3f vmm = v3f_translate_by_mat4(mm,   view);
    if (va.z < NEAR || vb.z < NEAR || vmp.z < NEAR || vmm.z < NEAR) return;

    Color col3[3] = {color, color, color};
    V3f   uvs[3]  = {0};
    renderer_push_triangle(va,  vmp, vmm, col3, uvs, NULL,
        TRIANGLE_WRITE_OVER_Z | TRIANGLE_NO_CULLING | TRIANGLE_WIRE_FRAME);
    renderer_push_triangle(vb,  vmp, vmm, col3, uvs, NULL,
        TRIANGLE_WRITE_OVER_Z | TRIANGLE_NO_CULLING | TRIANGLE_WIRE_FRAME);
}

// ---- AI-GENERATED: anim editor helpers -------------------------------------
// Review section: coordinate system note
//   model_to_world flips z, so world-space and model-space differ by z-sign.
//   Gizmo translation deltas (world-space) must have z negated before applying
//   to edit_pose[j].t (model-space).
//   Rotation about Y should be correct; X and Z rings may appear inverted —
//   test with a known rotation and adjust drag direction if needed.

// Compute world-space centroids for each skin group, used to position gizmos.
// AI-GENERATED
static void compute_joint_centers(void)
{
    Asset_Model *model = m_editor.selected;
    if (!model) return;

    V3f  sums[16]   = {0};
    u32  counts[16] = {0};
    memset(m_editor.joint_has_verts, 0, sizeof(m_editor.joint_has_verts));

    for (u32 i = 0; i < model->base_count; i++) {
        u16 g = model->skin_groups[i];
        if (g == ANIM_GROUP_STATIC || g >= 16) continue;
        sums[g] = v3f_add(sums[g], model->base_positions[i]);
        counts[g]++;
    }

    for (u16 g = 0; g < 16; g++) {
        if (counts[g] == 0) continue;
        V3f center = v3f_scale(sums[g], 1.0f / (f32)counts[g]);
        m_editor.joint_centers_model[g] = center;
        // Convert model-space centroid to world space (z-flip + model transform)
        m_editor.joint_centers[g] = model_to_world(center, m_editor.transform, v3f(0, 0, 0));
        m_editor.joint_has_verts[g] = true;
    }
}

// Build a flat FrameBase from all painted groups. Joint index == group index so
// anim_apply_pose can index world[] directly by group.
// Frees any previous edit_base / edit_anim.
// AI-GENERATED
static void anim_editor_build_skel(void)
{
    // Free previous data
    if (m_editor.edit_base) {
        arrfree(m_editor.edit_base->joints);
        free(m_editor.edit_base);
        m_editor.edit_base = NULL;
    }
    if (m_editor.edit_anim) {
        for (s32 i = 0; i < arrlen(m_editor.edit_anim->frames); i++)
            free(m_editor.edit_anim->frames[i].xforms);
        for (s32 i = 0; i < arrlen(m_editor.edit_anim->sequences); i++)
            arrfree(m_editor.edit_anim->sequences[i].frames);
        arrfree(m_editor.edit_anim->frames);
        arrfree(m_editor.edit_anim->sequences);
        free(m_editor.edit_anim);
        m_editor.edit_anim = NULL;
    }

    m_editor.edit_base     = calloc(1, sizeof(FrameBase));
    m_editor.edit_anim     = calloc(1, sizeof(AnimData));
    m_editor.selected_joint = -1;
    m_editor.gizmo_axis    = -1;

    // Find highest group index used so joints[g].group == g is guaranteed
    u16 max_group = 0;
    Asset_Model *model = m_editor.selected;
    for (u32 i = 0; i < model->base_count; i++) {
        u16 g = model->skin_groups[i];
        if (g != ANIM_GROUP_STATIC && g < 16 && g > max_group)
            max_group = g;
    }

    // One joint per group index 0..max_group; gaps get a placeholder entry
    for (u16 g = 0; g <= max_group; g++) {
        AnimJoint j = { .type = ANIM_XFORM_ROTATE, .group = g, .parent = -1 };
        arrput(m_editor.edit_base->joints, j);
    }
    m_editor.edit_base->count = (u32)arrlen(m_editor.edit_base->joints);
    m_editor.edit_anim->base  = m_editor.edit_base;

    // Reset all poses to identity
    for (s32 j = 0; j < 16; j++) {
        m_editor.edit_pose[j] = (Transform){
            .r = quat_identity(),
            .t = v3f(0, 0, 0),
            .s = v3f(1, 1, 1),
        };
        m_editor.joint_t_user[j] = v3f(0, 0, 0);
    }

    console_write_log_alloc("Built skel: %u joints", m_editor.edit_base->count);
}

// Snapshot the current edit_pose into a new AnimFrame and push it onto edit_anim.
// AI-GENERATED
static void anim_editor_add_frame(void)
{
    if (!m_editor.edit_base || !m_editor.edit_anim) return;
    u32 jcount = m_editor.edit_base->count;
    AnimFrame frame = {0};
    frame.xforms = malloc(sizeof(Transform) * jcount);
    // edit_pose is indexed by group; since joint index == group index, a direct
    // copy of the first jcount entries is correct.
    memcpy(frame.xforms, m_editor.edit_pose, sizeof(Transform) * jcount);
    arrput(m_editor.edit_anim->frames, frame);
    console_write_log_alloc("Frame %d added", (s32)arrlen(m_editor.edit_anim->frames) - 1);
}

// Write the current edit_base as a .skel file.
// AI-GENERATED
static void write_skel_file(const char *path)
{
    if (!m_editor.edit_base) return;
    FILE *f = fopen(path, "w");
    if (!f) { console_write_log_alloc("write_skel_file: failed to open %s", path); return; }
    fprintf(f, "%u\n", m_editor.edit_base->count);
    for (u32 j = 0; j < m_editor.edit_base->count; j++) {
        AnimJoint *jt = &m_editor.edit_base->joints[j];
        const char *type = (jt->type == ANIM_XFORM_TRANSLATE) ? "translate"
                         : (jt->type == ANIM_XFORM_SCALE)     ? "scale"
                                                               : "rotate";
        fprintf(f, "%s %hu %hd\n", type, jt->group, jt->parent);
    }
    fclose(f);
}

// Write all recorded frames plus a single looping sequence to a .anim file.
// AI-GENERATED
static void write_anim_file(const char *path)
{
    if (!m_editor.edit_anim || !m_editor.edit_base) return;
    s32 fcount = (s32)arrlen(m_editor.edit_anim->frames);
    if (fcount == 0) { console_write_log_alloc("No frames to save"); return; }

    FILE *f = fopen(path, "w");
    if (!f) { console_write_log_alloc("write_anim_file: failed to open %s", path); return; }

    u32 jcount = m_editor.edit_base->count;
    for (s32 fi = 0; fi < fcount; fi++) {
        AnimFrame *fr = &m_editor.edit_anim->frames[fi];
        fprintf(f, "frame\n");
        for (u32 j = 0; j < jcount; j++) {
            Transform *xf = &fr->xforms[j];
            fprintf(f, "%.6f %.6f %.6f %.6f  %.6f %.6f %.6f  %.6f %.6f %.6f\n",
                    xf->r.x, xf->r.y, xf->r.z, xf->r.w,
                    xf->t.x, xf->t.y, xf->t.z,
                    xf->s.x, xf->s.y, xf->s.z);
        }
        fprintf(f, "end\n");
    }

    const char *name = (m_editor.seq_name[0]) ? m_editor.seq_name : "idle";
    fprintf(f, "sequence %s loop\n", name);
    for (s32 fi = 0; fi < fcount; fi++)
        fprintf(f, "%d %u\n", fi, (u32)m_editor.frame_dur_ms);
    fprintf(f, "end\n");

    fclose(f);
    console_write_log_alloc("Saved anim -> %s  (%d frames)", path, fcount);
}

// Draw model triangles using live (post-pose) vertex positions and renderer.camera.
// Colour-codes by group so you can see group assignments while posing.
// AI-GENERATED — note: uses renderer.camera (not m_editor.camera) so the view
// matches the gizmo. If the model appears offset vs. PAINT mode, the two cameras differ.
static void draw_anim_groups_live(Asset_Model *model)
{
    Mat4  view = camera_matrix(renderer.camera);
    Color grey = {160, 160, 160, 255};

    for (s32 i = 0; i < arrlen(model->vertices); i += 3) {
        // model->vertices[j].position is model-space after anim_apply_pose.
        // Apply the same model→world transform used everywhere else.
        V3f p1 = model_to_world(model->vertices[i    ].position, m_editor.transform, v3f(0,0,0));
        V3f p2 = model_to_world(model->vertices[i + 1].position, m_editor.transform, v3f(0,0,0));
        V3f p3 = model_to_world(model->vertices[i + 2].position, m_editor.transform, v3f(0,0,0));
        p1 = v3f_translate_by_mat4(p1, view);
        p2 = v3f_translate_by_mat4(p2, view);
        p3 = v3f_translate_by_mat4(p3, view);
        if (p1.z < NEAR || p2.z < NEAR || p3.z < NEAR) continue;

        u16    g    = model->skin_groups[model->index_buffer[i]];
        Color  c    = (g == ANIM_GROUP_STATIC) ? grey : GROUP_PALETTE[g % 16];
        Color  col3[3] = {c, c, c};
        V3f    uvs[3]  = {0};
        renderer_push_triangle(p1, p2, p3, col3, uvs, NULL, TRIANGLE_WRITE_OVER_Z);
    }
}

// Load a recorded frame back into edit_pose and apply it as a live preview.
static void anim_editor_load_frame(s32 idx)
{
    if (!m_editor.edit_base || !m_editor.edit_anim) return;
    if (idx < 0 || idx >= (s32)arrlen(m_editor.edit_anim->frames)) return;
    u32 jcount = m_editor.edit_base->count;
    AnimFrame *fr = &m_editor.edit_anim->frames[idx];
    memcpy(m_editor.edit_pose, fr->xforms, sizeof(Transform) * jcount);
    // Recover user translations by un-baking pivot correction (t_stored = t_user + pivot - R*pivot)
    for (u32 j = 0; j < jcount; j++) {
        V3f pivot = m_editor.joint_centers_model[j];
        Quat Rq   = m_editor.edit_pose[j].r;
        m_editor.joint_t_user[j] = v3f_add(
            v3f_sub(m_editor.edit_pose[j].t, pivot),
            quat_rotate_v3f(Rq, pivot));
    }
    anim_apply_pose(m_editor.edit_base, m_editor.edit_pose, m_editor.selected);
    m_editor.selected_frame = idx;
}

// Overwrite a recorded frame with the current edit_pose.
static void anim_editor_update_frame(s32 idx)
{
    if (!m_editor.edit_base || !m_editor.edit_anim) return;
    if (idx < 0 || idx >= (s32)arrlen(m_editor.edit_anim->frames)) return;
    u32 jcount = m_editor.edit_base->count;
    AnimFrame *fr = &m_editor.edit_anim->frames[idx];
    memcpy(fr->xforms, m_editor.edit_pose, sizeof(Transform) * jcount);
    console_write_log_alloc("Frame %d updated", idx);
}

// Append a copy of an existing frame.
static void anim_editor_duplicate_frame(s32 idx)
{
    if (!m_editor.edit_base || !m_editor.edit_anim) return;
    if (idx < 0 || idx >= (s32)arrlen(m_editor.edit_anim->frames)) return;
    u32 jcount = m_editor.edit_base->count;
    AnimFrame src = m_editor.edit_anim->frames[idx];
    AnimFrame copy = { .xforms = malloc(sizeof(Transform) * jcount) };
    memcpy(copy.xforms, src.xforms, sizeof(Transform) * jcount);
    arrput(m_editor.edit_anim->frames, copy);
    console_write_log_alloc("Frame %d duplicated -> %d", idx,
                            (s32)arrlen(m_editor.edit_anim->frames) - 1);
}

// Delete a recorded frame and adjust selected_frame.
static void anim_editor_delete_frame(s32 idx)
{
    if (!m_editor.edit_anim) return;
    s32 fcount = (s32)arrlen(m_editor.edit_anim->frames);
    if (idx < 0 || idx >= fcount) return;
    free(m_editor.edit_anim->frames[idx].xforms);
    arrdel(m_editor.edit_anim->frames, idx);
    if (m_editor.selected_frame >= (s32)arrlen(m_editor.edit_anim->frames))
        m_editor.selected_frame = (s32)arrlen(m_editor.edit_anim->frames) - 1;
    console_write_log_alloc("Frame %d deleted", idx);
}

static void draw_anim_panel(void)
{
    struct nk_context *ui = platform_ctx.ui;
    static const nk_flags flags =
        NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_TITLE;

    if (!nk_begin(ui, "Anim Editor",
                  nk_rect(0, 0, 400, GAME_HEIGHT * 3 / 4), flags)) {
        nk_end(ui);
        return;
    }

    nk_layout_row_dynamic(ui, 18, 1);
    nk_label_wrap(ui, "Mode: ANIM  [G to change]");
    nk_label_wrap(ui, "Click node = select joint");
    nk_label_wrap(ui, "Drag node  = free move");
    nk_label_wrap(ui, "R = gizmo rotate/trans");

    nk_layout_row_dynamic(ui, 24, 1);
    if (nk_button_label(ui, m_editor.ik_mode ? ">IK ON  [I]" : " IK off [I]"))
        m_editor.ik_mode = !m_editor.ik_mode;
    if (nk_button_label(ui, m_editor.prefer_gizmo ? ">Gizmo mode" : " Free drag"))
        m_editor.prefer_gizmo = !m_editor.prefer_gizmo;

    //nk_layout_row_dynamic(ui, 24, 1);
    if (nk_button_label(ui, "Build Skel")) {
        anim_editor_build_skel();
        compute_joint_centers();
    }

    if (!m_editor.edit_base) {
        nk_end(ui);
        return;
    }

    nk_layout_row_dynamic(ui, 18, 1);
    char info[64];
    snprintf(info, sizeof(info), "Joints: %u  [R=rot/trans]", m_editor.edit_base->count);
    nk_label_wrap(ui, info);

    char label[8];
    nk_layout_row_dynamic(ui, 28, 4);
    for (u16 g = 0; g < 16; g++) {
        if (!m_editor.joint_has_verts[g]) continue;
        snprintf(label, sizeof(label), "%s%d",
                 (g == (u16)m_editor.selected_joint) ? ">" : " ", g);
        if (nk_button_label(ui, label)) {
            m_editor.selected_joint = (s32)g;
            m_editor.joint_gizmo.position        = m_editor.joint_centers[g];
            m_editor.joint_gizmo.attached         = true;
            m_editor.joint_gizmo.mode             = GIZMO_MODE_ROTATE;
            m_editor.joint_gizmo.axis_rotation[0] = 0.0f;
            m_editor.joint_gizmo.axis_rotation[1] = 0.0f;
            m_editor.joint_gizmo.axis_rotation[2] = 0.0f;
            m_editor.gizmo_axis                   = -1;
        }
    }

    if (m_editor.selected_joint >= 0) {
        s32 j = m_editor.selected_joint;
        s16 cur_parent = m_editor.edit_base->joints[j].parent;

        nk_layout_row_dynamic(ui, 18, 1);
        char par_hdr[48];
        snprintf(par_hdr, sizeof(par_hdr), "Joint %d parent:", j);
        nk_label_wrap(ui, par_hdr);

        char lbl[8];
        nk_layout_row_dynamic(ui, 24, 4);
        snprintf(lbl, sizeof(lbl), "%sNone", (cur_parent == -1) ? ">" : " ");
        if (nk_button_label(ui, lbl)) {
            m_editor.edit_base->joints[j].parent = -1;
            anim_apply_pose(m_editor.edit_base, m_editor.edit_pose, m_editor.selected);
        }
        for (s32 p = 0; p < j; p++) {
            if (!m_editor.joint_has_verts[p]) continue;
            snprintf(lbl, sizeof(lbl), "%s%d", (cur_parent == (s16)p) ? ">" : " ", p);
            if (nk_button_label(ui, lbl)) {
                m_editor.edit_base->joints[j].parent = (s16)p;
                anim_apply_pose(m_editor.edit_base, m_editor.edit_pose, m_editor.selected);
            }
        }
    }

    nk_layout_row_dynamic(ui, 24, 2);
    if (nk_button_label(ui, "Add Frame"))
        anim_editor_add_frame();
    if (nk_button_label(ui, "Clear Pose")) {
        for (s32 j = 0; j < 16; j++) {
            m_editor.edit_pose[j] = (Transform){
                .r = quat_identity(), .t = v3f(0,0,0), .s = v3f(1,1,1)
            };
            m_editor.joint_t_user[j] = v3f(0, 0, 0);
        }
        if (m_editor.edit_base)
            anim_apply_pose(m_editor.edit_base, m_editor.edit_pose, m_editor.selected);
    }

    s32 fcount = (s32)arrlen(m_editor.edit_anim->frames);
    nk_layout_row_dynamic(ui, 18, 1);
    snprintf(info, sizeof(info), "Frames: %d", fcount);
    nk_label_wrap(ui, info);

    if (fcount > 0) {
        nk_layout_row_dynamic(ui, 150, 1);
        if (nk_group_begin(ui, "frames", NK_WINDOW_BORDER)) {
            for (s32 fi = 0; fi < fcount; fi++) {
                b32 is_sel = (fi == m_editor.selected_frame);
                char fl[12];
                snprintf(fl, sizeof(fl), "%s%d", is_sel ? ">" : " ", fi);
                nk_layout_row_dynamic(ui, 22, 4);
                nk_label_wrap(ui, fl);
                if (nk_button_label(ui, "Load"))
                    anim_editor_load_frame(fi);
                if (nk_button_label(ui, "Upd") && is_sel)
                    anim_editor_update_frame(fi);
                if (nk_button_label(ui, "Del")) {
                    anim_editor_delete_frame(fi);
                    break;
                }
            }
            nk_group_end(ui);
        }
    }

    nk_layout_row_dynamic(ui, 24, 1);
    if (nk_button_label(ui, "Duplicate Frame") && m_editor.selected_frame >= 0)
        anim_editor_duplicate_frame(m_editor.selected_frame);

    nk_layout_row_dynamic(ui, 18, 1);
    nk_label_wrap(ui, "Sequence name:");
    nk_layout_row_dynamic(ui, 24, 1);
    nk_edit_string_zero_terminated(ui, NK_EDIT_FIELD, m_editor.seq_name,
                                   sizeof(m_editor.seq_name), nk_filter_default);

    nk_layout_row_dynamic(ui, 18, 1);
    nk_label_wrap(ui, "ms per frame:");
    nk_layout_row_dynamic(ui, 24, 1);
    nk_slider_float(ui, 50.0f, &m_editor.frame_dur_ms, 1000.0f, 1.0f);

    nk_layout_row_dynamic(ui, 24, 1);
    if (nk_button_label(ui, "Save Skel + Anim")) {
        char skel_path[300], anim_path[300];
        snprintf(skel_path, sizeof(skel_path), "data/%s.skel", m_editor.model_name);
        snprintf(anim_path, sizeof(anim_path), "data/%s.anim", m_editor.model_name);
        write_skel_file(skel_path);
        write_anim_file(anim_path);
    }

    nk_end(ui);
}

// ---- public API ------------------------------------------------------------

void model_editor_init(void)
{
    m_editor.arena     = arena_alloc();
    m_editor.transform = mat3_identity();
    m_editor.camera.front = v3f_normalize(
        v3f_sub(m_editor.camera.target, m_editor.camera.position));
}

void model_editor_load_model(String8 model_name)
{
    char *str = str8_to_cstring(m_editor.arena, model_name);
    snprintf(m_editor.model_name, sizeof(m_editor.model_name), "%s", str);
    m_editor.selected = get_model(str);
    console_write_log_alloc("Loaded model %s", str);

    u32 vert_count = (u32)arrlen(m_editor.selected->vertices);
    m_editor.selected_triangles = push_array(m_editor.arena, b32, vert_count);
    memset(m_editor.selected_triangles, 0, sizeof(b32) * vert_count);

    // Pre-bake world-space positions so draw_skin_groups and pick_triangle
    // don't recompute mat3 + Z-flip + add every frame.
    m_editor.world_positions = push_array(m_editor.arena, V3f, vert_count);
    for (u32 i = 0; i < vert_count; i++)
        m_editor.world_positions[i] = model_to_world(
            m_editor.selected->vertices[i].position, m_editor.transform, v3f(0,0,0));
}

static void dispatch_ui_commands(void)
{
    const struct nk_command *cmd;
    f32 ui_z = 0.4f;
    nk_foreach(cmd, platform_ctx.ui) {
        switch (cmd->type) {
            case NK_COMMAND_RECT_FILLED: {
                const struct nk_command_rect_filled *r =
                    (const struct nk_command_rect_filled *)cmd;
                Color c = {r->color.r, r->color.g, r->color.b, r->color.a};
                draw_recs32((Recs32){(f32)r->x, (f32)r->y, (f32)r->w, (f32)r->h}, ui_z, c);
                ui_z -= 0.001f;
            } break;
            case NK_COMMAND_TEXT: {
                const struct nk_command_text *t = (const struct nk_command_text *)cmd;
                Color c = {t->foreground.r, t->foreground.g, t->foreground.b, t->foreground.a};
                draw_text(m_editor.hud_font, t->string, v2i(t->x, t->y), (s32)t->height, c);
            } break;
            default: break;
        }
    }
    nk_clear(platform_ctx.ui);
}

// ---- rig editor helpers ----------------------------------------------------

// Place a new joint at a world-space position, parented to `parent` (-1 = root).
static void rig_add_joint(V3f world_pos, s16 parent)
{
    if (!m_editor.edit_base) {
        m_editor.edit_base = calloc(1, sizeof(FrameBase));
        if (!m_editor.edit_anim) {
            m_editor.edit_anim = calloc(1, sizeof(AnimData));
        }
        m_editor.edit_anim->base = m_editor.edit_base;
    }
    u32 g = m_editor.edit_base->count;
    if (g >= 16) { console_write_log_alloc("Max 16 joints reached"); return; }

    AnimJoint j = { .type = ANIM_XFORM_ROTATE, .group = (u16)g, .parent = parent };
    arrput(m_editor.edit_base->joints, j);
    m_editor.edit_base->count++;

    m_editor.joint_centers[g]   = world_pos;
    m_editor.joint_active[g]    = true;
    m_editor.joint_has_verts[g] = false;
    m_editor.edit_pose[g] = (Transform){
        .r = quat_identity(), .t = v3f(0,0,0), .s = v3f(1,1,1)
    };
    m_editor.selected_joint = (s32)g;
    console_write_log_alloc("Joint %u placed", g);
}

// Remove joint g, renumbering all subsequent joint indices and parent refs.
static void rig_delete_joint(s32 g)
{
    if (!m_editor.edit_base || g < 0 || g >= (s32)m_editor.edit_base->count) return;

    arrdel(m_editor.edit_base->joints, g);
    m_editor.edit_base->count--;

    // Fix parent references
    for (u32 j = 0; j < m_editor.edit_base->count; j++) {
        s16 p = m_editor.edit_base->joints[j].parent;
        if      (p == (s16)g) m_editor.edit_base->joints[j].parent = -1;
        else if (p  > (s16)g) m_editor.edit_base->joints[j].parent--;
        // Also fix the group index stored in the joint
        if ((s32)m_editor.edit_base->joints[j].group > g)
            m_editor.edit_base->joints[j].group--;
    }

    // Shift joint_centers, joint_active, joint_has_verts down
    for (s32 i = g; i < 15; i++) {
        m_editor.joint_centers[i]   = m_editor.joint_centers[i+1];
        m_editor.joint_active[i]    = m_editor.joint_active[i+1];
        m_editor.joint_has_verts[i] = m_editor.joint_has_verts[i+1];
        m_editor.edit_pose[i]       = m_editor.edit_pose[i+1];
    }
    m_editor.joint_active[15]    = false;
    m_editor.joint_has_verts[15] = false;

    // Shift skin group assignments on the model
    if (m_editor.selected) {
        for (u32 i = 0; i < m_editor.selected->base_count; i++) {
            u16 sg = m_editor.selected->skin_groups[i];
            if (sg == (u16)g)
                m_editor.selected->skin_groups[i] = ANIM_GROUP_STATIC;
            else if (sg != ANIM_GROUP_STATIC && sg > (u16)g)
                m_editor.selected->skin_groups[i]--;
        }
    }

    if (m_editor.selected_joint == g)        m_editor.selected_joint = -1;
    else if (m_editor.selected_joint > g)    m_editor.selected_joint--;
    console_write_log_alloc("Joint %d removed", g);
}

// Assign each base vertex to the nearest joint by world-space distance.
static void rig_auto_skin(void)
{
    if (!m_editor.edit_base || !m_editor.selected) return;
    u32 jcount = m_editor.edit_base->count;
    if (jcount == 0) return;

    for (u32 i = 0; i < m_editor.selected->base_count; i++) {
        V3f pw = model_to_world(m_editor.selected->base_positions[i],
                                m_editor.transform, v3f(0,0,0));
        f32 best = FLT_MAX;
        u16 best_g = ANIM_GROUP_STATIC;
        for (u16 g = 0; g < jcount && g < 16; g++) {
            V3f d = v3f_sub(pw, m_editor.joint_centers[g]);
            f32 dist = sqrtf(v3f_dot(d, d));
            if (dist < best) { best = dist; best_g = g; }
        }
        m_editor.selected->skin_groups[i] = best_g;
    }
    compute_joint_centers();
    console_write_log_alloc("Auto-skinned %u verts to %u joints",
                            m_editor.selected->base_count, jcount);
}

// Wipe the rig to zero joints without touching the anim data.
static void rig_clear_skeleton(void)
{
    if (m_editor.edit_base) {
        arrfree(m_editor.edit_base->joints);
        m_editor.edit_base->count = 0;
    } else {
        m_editor.edit_base = calloc(1, sizeof(FrameBase));
        if (m_editor.edit_anim) m_editor.edit_anim->base = m_editor.edit_base;
    }
    m_editor.selected_joint = -1;
    memset(m_editor.joint_active,    0, sizeof(m_editor.joint_active));
    memset(m_editor.joint_has_verts, 0, sizeof(m_editor.joint_has_verts));
    console_write_log_alloc("Skeleton cleared");
}

static void draw_rig_panel(void)
{
    struct nk_context *ui = platform_ctx.ui;
    static const nk_flags flags =
        NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_TITLE;

    if (!nk_begin(ui, "Rig Editor",
                  nk_rect(0, 0, 400, GAME_HEIGHT * 3 / 4), flags)) {
        nk_end(ui);
        return;
    }

    nk_layout_row_dynamic(ui, 80, 1);
    nk_label_wrap(ui, "Mode: RIG [G to change]");
    nk_label_wrap(ui, "Click mesh = place joint");
    nk_label_wrap(ui, "Drag node  = move joint");
    nk_label_wrap(ui, "Ctrl+click = set parent");
    nk_label_wrap(ui, "Del        = remove joint");

    nk_layout_row_dynamic(ui, 24, 1);
    if (nk_button_label(ui, "Auto-skin to joints"))
        rig_auto_skin();

    nk_layout_row_dynamic(ui, 24, 1);
    if (nk_button_label(ui, "Clear Skeleton"))
        rig_clear_skeleton();

    if (!m_editor.edit_base || m_editor.edit_base->count == 0) {
        nk_end(ui);
        return;
    }

    char info[64];
    snprintf(info, sizeof(info), "Joints: %u", m_editor.edit_base->count);
    nk_layout_row_dynamic(ui, 18, 1);
    nk_label_wrap(ui, info);

    nk_layout_row_dynamic(ui, 120, 1);
    if (nk_group_begin(ui, "rig_joints", NK_WINDOW_BORDER)) {
        for (u32 g = 0; g < m_editor.edit_base->count; g++) {
            b32 is_sel = ((s32)g == m_editor.selected_joint);
            s16 par    = m_editor.edit_base->joints[g].parent;
            char row[48];
            if (par < 0)
                snprintf(row, sizeof(row), "%sJ%u (root)", is_sel ? ">" : " ", g);
            else
                snprintf(row, sizeof(row), "%sJ%u -> J%d", is_sel ? ">" : " ", g, (s32)par);
            nk_layout_row_dynamic(ui, 20, 1);
            if (nk_button_label(ui, row))
                m_editor.selected_joint = (s32)g;
        }
        nk_group_end(ui);
    }

    if (m_editor.selected_joint >= 0) {
        nk_layout_row_dynamic(ui, 24, 1);
        char del_lbl[32];
        snprintf(del_lbl, sizeof(del_lbl), "Remove Joint %d", m_editor.selected_joint);
        if (nk_button_label(ui, del_lbl))
            rig_delete_joint(m_editor.selected_joint);
    }

    nk_end(ui);
}

void model_editor_rig_frame(f32 dt)
{
    UNUSED(dt);
    if (!m_editor.selected) return;

    renderer.camera = m_editor.camera;

    V2f mouse       = get_mouse_pos();
    V2f mouse_delta = get_mouse_delta();
    Ray mouse_ray   = get_mouse_ray(renderer.camera, mouse);
    UNUSED(mouse);

    bool mouse_pressed  = is_mouse_button_pressed(MOUSEBUTTON_LEFT);
    bool mouse_down     = is_mouse_button_down(MOUSEBUTTON_LEFT);
    bool mouse_released = is_mouse_button_released(MOUSEBUTTON_LEFT);
    bool ctrl_held      = is_key_down(KEY_LEFT_CONTROL);
    bool ui_over        = nk_window_is_any_hovered(platform_ctx.ui);

    if (is_key_pressed(KEY_DELETE) && m_editor.selected_joint >= 0)
        rig_delete_joint(m_editor.selected_joint);

    if (mouse_pressed && !ui_over) {
        // Check if clicking an existing joint node
        s32 picked = -1;
        f32 closest = FLT_MAX;
        if (m_editor.edit_base) {
            for (u32 g = 0; g < m_editor.edit_base->count && g < 16; g++) {
                f32 r = 0.15f;
                AABB nb = {
                    .min = v3f_sub(m_editor.joint_centers[g], v3f(r,r,r)),
                    .max = v3f_add(m_editor.joint_centers[g], v3f(r,r,r))
                };
                RayCollision col = get_raycollision_box(mouse_ray, nb);
                if (col.hit && col.distance < closest) {
                    closest = col.distance;
                    picked  = (s32)g;
                }
            }
        }

        if (picked >= 0) {
            if (ctrl_held && m_editor.selected_joint >= 0 &&
                picked != m_editor.selected_joint &&
                m_editor.edit_base)
            {
                // Ctrl+click another joint: set it as parent of the selected joint.
                // Parent must have a lower index to preserve topological order.
                if (picked < m_editor.selected_joint) {
                    m_editor.edit_base->joints[m_editor.selected_joint].parent = (s16)picked;
                    console_write_log_alloc("Joint %d parent -> %d",
                                            m_editor.selected_joint, picked);
                } else {
                    console_write_log_alloc("Parent must have lower index than child");
                }
            } else {
                m_editor.selected_joint = picked;
                m_editor.node_dragging  = true;
            }
        } else if (!ctrl_held) {
            // Click on empty space / model surface: place a new joint
            V3f hit = v3f_add(mouse_ray.position, v3f_scale(mouse_ray.direction, 3.0f));
            f32 best_dist = FLT_MAX;
            for (s32 i = 0; i < arrlen(m_editor.selected->vertices); i += 3) {
                RayCollision col = get_ray_collision_triangle(
                    mouse_ray,
                    m_editor.world_positions[i],
                    m_editor.world_positions[i+1],
                    m_editor.world_positions[i+2]);
                if (col.hit && col.distance < best_dist) {
                    best_dist = col.distance;
                    hit       = col.point;
                }
            }
            // Parent the new joint to whatever is currently selected
            s16 parent = (s16)m_editor.selected_joint;
            // Validate: new joint will be index == current count, so parent must be < that
            if (m_editor.edit_base &&
                parent >= (s16)m_editor.edit_base->count)
                parent = -1;
            rig_add_joint(hit, parent);
        }
    }

    // Drag selected joint
    if (m_editor.node_dragging && mouse_down && m_editor.selected_joint >= 0) {
        s32 j = m_editor.selected_joint;
        Camera cam = renderer.camera;
        V3f right  = v3f_normalize(v3f_cross(cam.up, cam.front));
        V3f up_cam = v3f_normalize(v3f_cross(right, cam.front));
        V3f to_jt  = v3f_sub(m_editor.joint_centers[j], cam.position);
        f32 depth  = fabsf(v3f_dot(to_jt, cam.front));
        if (depth < 0.01f) depth = 0.01f;
        f32 scale  = depth * tanf(cam.fovy * 0.5f * (3.14159f / 180.0f)) * 2.0f / SCREEN_HEIGHT * 0.25f;
        V3f world_delta = v3f_add(v3f_scale(right,  mouse_delta.x *  scale),
                                  v3f_scale(up_cam, -mouse_delta.y * scale));
        m_editor.joint_centers[j] = v3f_add(m_editor.joint_centers[j], world_delta);
    }

    if (mouse_released)
        m_editor.node_dragging = false;

    // Draw model with current skin groups so user can see the assignments
    draw_skin_groups(m_editor.selected, m_editor.transform);

    // Hover detection for node highlight
    s32 hovered = -1;
    if (m_editor.edit_base && !m_editor.node_dragging) {
        f32 closest = FLT_MAX;
        for (u32 g = 0; g < m_editor.edit_base->count && g < 16; g++) {
            f32 r = 0.15f;
            AABB nb = {
                .min = v3f_sub(m_editor.joint_centers[g], v3f(r,r,r)),
                .max = v3f_add(m_editor.joint_centers[g], v3f(r,r,r))
            };
            RayCollision col = get_raycollision_box(mouse_ray, nb);
            if (col.hit && col.distance < closest) {
                closest = col.distance;
                hovered = (s32)g;
            }
        }
    }

    // Draw joint nodes and bones
    if (m_editor.edit_base) {
        Mat4 view = camera_matrix(renderer.camera);
        for (u32 g = 0; g < m_editor.edit_base->count && g < 16; g++) {
            V3f p = v3f_translate_by_mat4(m_editor.joint_centers[g], view);
            if (p.z < NEAR) continue;
            b32 is_sel = ((s32)g == m_editor.selected_joint);
            b32 is_hov = ((s32)g == hovered);
            Color c = is_sel ? (Color){255,255,255,255}
                    : is_hov ? (Color){255,255,100,255}
                             : GROUP_PALETTE[g % 16];
            Color col3[3] = {c,c,c};
            V3f   uvs[3]  = {0};
            f32 s = is_sel ? 0.05f : is_hov ? 0.038f : 0.028f;
            u32 flags = TRIANGLE_WRITE_OVER_Z | TRIANGLE_NO_CULLING | TRIANGLE_WIRE_FRAME;
            renderer_push_triangle(
                v3f(p.x, p.y+s, p.z), v3f(p.x-s, p.y-s, p.z), v3f(p.x+s, p.y-s, p.z),
                col3, uvs, NULL, flags);

            // Draw bone to parent
            s16 par = m_editor.edit_base->joints[g].parent;
            if (par >= 0 && (u32)par < m_editor.edit_base->count) {
                Color bc = is_sel ? (Color){255,255,200,200} : (Color){200,200,200,180};
                draw_bone_line(m_editor.joint_centers[(u16)par],
                               m_editor.joint_centers[g], bc);
            }
        }
    }

    draw_rig_panel();
    dispatch_ui_commands();
}

void model_editor_frame(void)
{
    if (!m_editor.selected) return;

    // AI-GENERATED: sync renderer.camera ← m_editor.camera so that
    // draw_rectangle3d (used by the gizmo) uses the same view as the model mesh.
    renderer.camera = m_editor.camera;
    model_editor_camera_update();

    // G cycles SELECT → PAINT → ANIM → SELECT.
    // When leaving ANIM, reset vertices to rest pose so world_positions cache is valid.
    if (is_key_pressed(KEY_G)) {
        if (m_editor.mode == MODEL_EDITOR_MODE_ANIM && m_editor.selected) {
            // AI-GENERATED: restore rest pose on exit from ANIM mode
            u32 flat = (u32)arrlen(m_editor.selected->vertices);
            for (u32 j = 0; j < flat; j++)
                m_editor.selected->vertices[j].position =
                    m_editor.selected->base_positions[m_editor.selected->index_buffer[j]];
        }
        switch (m_editor.mode) {
            case MODEL_EDITOR_MODE_SELECT: m_editor.mode = MODEL_EDITOR_MODE_PAINT;  break;
            case MODEL_EDITOR_MODE_PAINT:  m_editor.mode = MODEL_EDITOR_MODE_RIG;    break;
            case MODEL_EDITOR_MODE_RIG:    m_editor.mode = MODEL_EDITOR_MODE_ANIM;   break;
            case MODEL_EDITOR_MODE_ANIM:   m_editor.mode = MODEL_EDITOR_MODE_SELECT; break;
        }
        m_editor.hovered_tri = -1;
    }

    // RIG mode: bone placement and editing.
    if (m_editor.mode == MODEL_EDITOR_MODE_RIG) {
        model_editor_rig_frame(renderer.dt * 1000.0f);
        return;
    }

    // ANIM mode: hand off to the dedicated frame function and show only its panel.
    if (m_editor.mode == MODEL_EDITOR_MODE_ANIM) {
        model_editor_anim_frame(renderer.dt * 1000.0f);
        return;
    }

    V2f mouse       = get_mouse_pos();
    V2f mouse_delta = get_mouse_delta();
    Ray m_ray       = get_mouse_ray(m_editor.camera, mouse);
    UNUSED(mouse_delta);

    // [ / ] cycle the active group
    if (is_key_pressed(KEY_LEFT_BRACKET))
        m_editor.active_group = (u16)((m_editor.active_group + 15) % 16);
    if (is_key_pressed(KEY_RIGHT_BRACKET))
        m_editor.active_group = (u16)((m_editor.active_group +  1) % 16);

    if (m_editor.mode == MODEL_EDITOR_MODE_SELECT) {
        draw_model_triangle_selection(m_editor.selected, v3f(0,0,0),
                                      m_editor.transform, m_editor.selected_triangles);
    } else {
        // PAINT mode
        draw_skin_groups(m_editor.selected, m_editor.transform);

        if (mouse.x != m_editor.last_pick_mouse.x ||
            mouse.y != m_editor.last_pick_mouse.y)
        {
            m_editor.hovered_tri     = pick_triangle(m_editor.selected, m_ray);
            m_editor.last_pick_mouse = mouse;
        }
        draw_hovered_tri(m_editor.selected, m_editor.transform, m_editor.hovered_tri);

        // Box paint: press starts anchor, drag >5px activates box mode.
        // Release in box mode paints the region; a plain click paints one triangle.
        bool ui_over = nk_window_is_any_hovered(platform_ctx.ui);
        if (!ui_over && is_mouse_button_pressed(MOUSEBUTTON_LEFT)) {
            m_editor.box_paint_start = mouse;
            m_editor.box_painting    = false;
        }
        if (!ui_over && is_mouse_button_down(MOUSEBUTTON_LEFT)) {
            f32 dx = mouse.x - m_editor.box_paint_start.x;
            f32 dy = mouse.y - m_editor.box_paint_start.y;
            if (!m_editor.box_painting && (dx*dx + dy*dy) > 25.0f)
                m_editor.box_painting = true;

            if (m_editor.box_painting) {
                // Draw box overlay
                Color box_c = {255, 255, 100, 120};
                V2f s = m_editor.box_paint_start, e = mouse;
                f32 bx = fminf(s.x, e.x), by = fminf(s.y, e.y);
                f32 bw = fabsf(e.x - s.x),  bh = fabsf(e.y - s.y);
                draw_recs32((Recs32){bx, by, bw, bh}, 0.01f, box_c);
            } else if (m_editor.hovered_tri >= 0) {
                paint_triangle(m_editor.selected, m_editor.hovered_tri, m_editor.active_group);
            }
        }
        if (!ui_over && is_mouse_button_released(MOUSEBUTTON_LEFT) && m_editor.box_painting) {
            box_paint_region(m_editor.selected, m_editor.box_paint_start, mouse, m_editor.active_group);
            m_editor.box_painting = false;
        }
        if (is_key_pressed(KEY_F) && m_editor.hovered_tri >= 0)
            flood_fill_paint(m_editor.selected, m_editor.hovered_tri, m_editor.active_group);
    }

    draw_skin_panel();
    dispatch_ui_commands();
}

// AI-GENERATED: gizmo-based keyframe posing.
//
// Workflow:
//   1. Paint skin groups (PAINT mode), then press G to reach ANIM mode.
//   2. Click "Build Skel from Groups" in the panel.
//   3. Click a joint button to select it — a rotation gizmo appears at its centroid.
//   4. Drag a ring to rotate that group's vertices.  R toggles rotate ↔ translate.
//   5. "Add Frame" records the current pose.  Repeat for each keyframe.
//   6. "Save Skel + Anim" writes data/<model>.skel and data/<model>.anim.
//
// Review notes:
//   • renderer.camera is synced from m_editor.camera at the top of model_editor_frame,
//     so gizmo (draw_rectangle3d) and model mesh use the same view.
//   • Gizmo translation delta is world-space; z is negated before storing in edit_pose
//     because model_to_world flips z.  X/Z rotation rings may appear inverted — drag
//     in the opposite direction if the deformation goes the wrong way.
void model_editor_anim_frame(f32 dt)
{
    UNUSED(dt);
    if (!m_editor.selected) return;

    V2f mouse         = get_mouse_pos();
    V2f mouse_delta   = get_mouse_delta();
    // renderer.camera was synced from m_editor.camera by model_editor_frame
    Ray mouse_ray     = get_mouse_ray(renderer.camera, mouse);
    UNUSED(mouse);

    bool mouse_pressed  = is_mouse_button_pressed(MOUSEBUTTON_LEFT);
    bool mouse_down     = is_mouse_button_down(MOUSEBUTTON_LEFT);
    bool mouse_released = is_mouse_button_released(MOUSEBUTTON_LEFT);
    bool ui_over        = nk_window_is_any_hovered(platform_ctx.ui);

    // R toggles rotate ↔ translate on the active joint's gizmo
    if (is_key_pressed(KEY_R) && m_editor.selected_joint >= 0) {
        m_editor.joint_gizmo.mode =
            (m_editor.joint_gizmo.mode == GIZMO_MODE_TRANSLATE)
            ? GIZMO_MODE_ROTATE
            : GIZMO_MODE_TRANSLATE;
        m_editor.gizmo_axis = -1;
    }

    // I toggles IK mode
    if (is_key_pressed(KEY_I))
        m_editor.ik_mode = !m_editor.ik_mode;

    // Rebuild AABBs every frame so they track the gizmo position
    if (m_editor.selected_joint >= 0)
        gizmo_update(&m_editor.joint_gizmo);

    // ---- mouse press: try to click a joint node directly in the viewport ----
    if (mouse_pressed && !ui_over && m_editor.edit_base) {
        f32 closest_node = FLT_MAX;
        s32 picked = -1;
        for (u16 g = 0; g < m_editor.edit_base->count; g++) {
            if (!m_editor.joint_has_verts[g]) continue;
            f32 r = 0.15f;
            AABB nb = {
                .min = v3f_sub(m_editor.joint_centers[g], v3f(r,r,r)),
                .max = v3f_add(m_editor.joint_centers[g], v3f(r,r,r))
            };
            RayCollision col = get_raycollision_box(mouse_ray, nb);
            if (col.hit && col.distance < closest_node) {
                closest_node = col.distance;
                picked = (s32)g;
            }
        }
        if (picked >= 0) {
            m_editor.selected_joint              = picked;
            m_editor.joint_gizmo.position        = m_editor.joint_centers[picked];
            m_editor.joint_gizmo.attached         = true;
            m_editor.joint_gizmo.mode             = GIZMO_MODE_ROTATE;
            m_editor.joint_gizmo.axis_rotation[0] = 0.0f;
            m_editor.joint_gizmo.axis_rotation[1] = 0.0f;
            m_editor.joint_gizmo.axis_rotation[2] = 0.0f;
            m_editor.gizmo_axis                   = -1;
            m_editor.node_dragging                = !m_editor.prefer_gizmo;
            m_editor.ik_drag_target               = m_editor.joint_centers[picked];
        }
    }

    // ---- mouse press: pick a gizmo axis (only if we didn't grab a node) ----
    if (mouse_pressed && !ui_over && !m_editor.node_dragging && m_editor.selected_joint >= 0) {
        f32 closest = FLT_MAX;
        m_editor.gizmo_axis = -1;
        for (s32 i = 0; i < GIZMO_AXIS_COUNT; i++) {
            RayCollision col = get_raycollision_box(mouse_ray, m_editor.joint_gizmo.aabbs[i]);
            if (col.hit && col.distance < closest) {
                closest = col.distance;
                m_editor.gizmo_axis = i;
            }
        }
    }

    // ---- node drag: free movement projected onto camera-perpendicular plane ----
    if (m_editor.node_dragging && mouse_down && m_editor.selected_joint >= 0) {
        s32 j = m_editor.selected_joint;
        Camera cam = renderer.camera;
        V3f right  = v3f_normalize(v3f_cross(cam.up, cam.front));
        V3f up_cam = v3f_normalize(v3f_cross(right, cam.front));
        V3f to_jt  = v3f_sub(m_editor.joint_centers[j], cam.position);
        f32 depth  = fabsf(v3f_dot(to_jt, cam.front));
        if (depth < 0.01f) depth = 0.01f;
        f32 scale  = depth * tanf(cam.fovy * 0.5f * (3.14159f / 180.0f)) * 2.0f / SCREEN_HEIGHT * 0.25f;
        V3f world_delta = v3f_add(v3f_scale(right,  mouse_delta.x *  scale),
                                  v3f_scale(up_cam, mouse_delta.y * scale));

        m_editor.ik_drag_target = v3f_add(m_editor.ik_drag_target, world_delta);
        V3f model_delta = v3f(world_delta.x, world_delta.y, -world_delta.z);
        m_editor.joint_t_user[j] = v3f_add(m_editor.joint_t_user[j], model_delta);
        {
            Quat Rq   = m_editor.edit_pose[j].r;
            V3f pivot = m_editor.joint_centers_model[j];
            m_editor.edit_pose[j].t = v3f_add(m_editor.joint_t_user[j],
                                               v3f_sub(pivot, quat_rotate_v3f(Rq, pivot)));
        }
        m_editor.joint_gizmo.position = v3f_add(m_editor.joint_gizmo.position, world_delta);

        // IK: rotate immediate parent to aim toward the dragged target
        if (m_editor.ik_mode && m_editor.edit_base) {
            s16 p = m_editor.edit_base->joints[j].parent;
            if (p >= 0 && m_editor.joint_has_verts[(u16)p]) {
                V3f pc  = m_editor.joint_centers[(u16)p];
                V3f jc  = m_editor.joint_centers[j];
                V3f diff = v3f_sub(jc, pc);
                f32 rest_len = sqrtf(v3f_dot(diff, diff));
                V3f to_tgt   = v3f_sub(m_editor.ik_drag_target, pc);
                f32 tgt_len  = sqrtf(v3f_dot(to_tgt, to_tgt));
                if (rest_len > 0.001f && tgt_len > 0.001f) {
                    V3f rest_dir_w = v3f_scale(diff,   1.0f / rest_len);
                    V3f tgt_dir_w  = v3f_scale(to_tgt, 1.0f / tgt_len);
                    Quat dq_w = quat_from_to(rest_dir_w, tgt_dir_w);
                    // Convert world→model space: z-flip negates quat's z axis component
                    Quat dq_m = quat_normalise((Quat){dq_w.x, dq_w.y, -dq_w.z, dq_w.w});
                    m_editor.edit_pose[(s32)p].r = dq_m;
                    V3f pp    = m_editor.joint_centers_model[(s32)p];
                    m_editor.edit_pose[(s32)p].t = v3f_add(m_editor.joint_t_user[(s32)p],
                                                            v3f_sub(pp, quat_rotate_v3f(dq_m, pp)));
                }
            }
        }

        if (m_editor.edit_base)
            anim_apply_pose(m_editor.edit_base, m_editor.edit_pose, m_editor.selected);
    }

    // ---- gizmo axis drag: precise axis-aligned rotate/translate ----
    if (m_editor.gizmo_axis >= 0 && mouse_down && m_editor.selected_joint >= 0) {
        s32 j = m_editor.selected_joint;

        if (m_editor.joint_gizmo.mode == GIZMO_MODE_ROTATE) {
            f32 angle = 0.0f;
            switch (m_editor.gizmo_axis) {
                case GIZMO_AXIS_X: angle = -mouse_delta.y * 0.003f; break;
                case GIZMO_AXIS_Y: angle =  mouse_delta.x * 0.003f; break;
                case GIZMO_AXIS_Z: angle = -mouse_delta.x * 0.003f; break;
                default: break;
            }
            gizmo_rotation_modify(&m_editor.joint_gizmo, (Gizmo_Axis)m_editor.gizmo_axis, angle);
            {
                Quat Rq   = mat3_to_quat(gizmo_get_rotation(&m_editor.joint_gizmo));
                V3f pivot = m_editor.joint_centers_model[j];
                m_editor.edit_pose[j].r = Rq;
                m_editor.edit_pose[j].t = v3f_add(m_editor.joint_t_user[j],
                                                   v3f_sub(pivot, quat_rotate_v3f(Rq, pivot)));
            }
        } else {
            V3f world_delta = gizmo_translation_modify(
                &m_editor.joint_gizmo, (Gizmo_Axis)m_editor.gizmo_axis,
                v2f_scale(mouse_delta, 0.003f));
            V3f model_delta = v3f(world_delta.x, world_delta.y, -world_delta.z);
            m_editor.joint_t_user[j] = v3f_add(m_editor.joint_t_user[j], model_delta);
            {
                Quat Rq   = m_editor.edit_pose[j].r;
                V3f pivot = m_editor.joint_centers_model[j];
                m_editor.edit_pose[j].t = v3f_add(m_editor.joint_t_user[j],
                                                   v3f_sub(pivot, quat_rotate_v3f(Rq, pivot)));
            }
            m_editor.joint_gizmo.position = v3f_add(m_editor.joint_gizmo.position, world_delta);
        }

        if (m_editor.edit_base)
            anim_apply_pose(m_editor.edit_base, m_editor.edit_pose, m_editor.selected);
    }

    if (mouse_released) {
        m_editor.gizmo_axis   = -1;
        m_editor.node_dragging = false;
    }

    // ---- draw model with group colours using live vertex positions ----
    draw_anim_groups_live(m_editor.selected);

    // ---- draw small triangle markers at joint centres ----
    // Markers are pushed as view-space triangles (renderer.camera already applied by
    // draw_anim_groups_live logic above, but markers go through renderer_push_triangle
    // which expects view-space).
    // Check which joint the mouse ray is nearest (for hover highlight)
    s32 hovered_joint = -1;
    if (m_editor.edit_base && !m_editor.node_dragging) {
        f32 closest_hover = FLT_MAX;
        for (u16 g = 0; g < m_editor.edit_base->count; g++) {
            if (!m_editor.joint_has_verts[g]) continue;
            f32 r = 0.15f;
            AABB nb = {
                .min = v3f_sub(m_editor.joint_centers[g], v3f(r,r,r)),
                .max = v3f_add(m_editor.joint_centers[g], v3f(r,r,r))
            };
            RayCollision col = get_raycollision_box(mouse_ray, nb);
            if (col.hit && col.distance < closest_hover) {
                closest_hover = col.distance;
                hovered_joint = (s32)g;
            }
        }
    }

    Mat4 view = camera_matrix(renderer.camera);
    for (u16 g = 0; g < 16; g++) {
        if (!m_editor.joint_has_verts[g]) continue;
        V3f p = v3f_translate_by_mat4(m_editor.joint_centers[g], view);
        if (p.z < NEAR) continue;
        b32 is_sel    = ((s32)g == m_editor.selected_joint);
        b32 is_hovered = ((s32)g == hovered_joint);
        Color c = is_sel     ? (Color){255, 255, 255, 255}
                : is_hovered ? (Color){255, 255, 100, 255}
                             : GROUP_PALETTE[g % 16];
        Color col3[3] = {c, c, c};
        V3f   uvs3[3] = {0};
        // Selected joint gets a larger marker; hovering joint gets medium
        f32 s = is_sel ? 0.045f : is_hovered ? 0.035f : 0.025f;
        u32 flags = TRIANGLE_WRITE_OVER_Z | TRIANGLE_NO_CULLING;
        if (is_sel || is_hovered) flags |= TRIANGLE_WIRE_FRAME;
        renderer_push_triangle(
            v3f(p.x,     p.y + s, p.z),
            v3f(p.x - s, p.y - s, p.z),
            v3f(p.x + s, p.y - s, p.z),
            col3, uvs3, NULL, flags);
    }

    // ---- draw bones between parent-child joint pairs ----
    if (m_editor.edit_base) {
        for (u32 g = 0; g < m_editor.edit_base->count; g++) {
            s16 p = m_editor.edit_base->joints[g].parent;
            if (p < 0 || !m_editor.joint_has_verts[g] || !m_editor.joint_has_verts[(u16)p])
                continue;
            Color bc = {220, 220, 220, 200};
            draw_bone_line(m_editor.joint_centers[(u16)p], m_editor.joint_centers[g], bc);
        }
    }

    // ---- draw gizmo if a joint is selected ----
    if (m_editor.selected_joint >= 0) {
        m_editor.joint_gizmo.active_axis = m_editor.gizmo_axis;
        gizmo_draw(&m_editor.joint_gizmo);
    }

    draw_anim_panel();
    dispatch_ui_commands();
}

void model_editor_camera_update(void)
{
	V2f mouse_delta = get_mouse_delta();
    f32 dt = renderer.dt;

    if (is_mouse_button_down(MOUSEBUTTON_MIDDLE)) {
        if (is_key_down(KEY_W)) {
            m_editor.camera.position = v3f_add(m_editor.camera.position, v3f_scale(m_editor.camera.front, 0.5 * dt));
        }
        if (is_key_down(KEY_S)) {
            m_editor.camera.position = v3f_add(m_editor.camera.position, v3f_scale(m_editor.camera.front, -0.5 * dt));
        }
        if (is_key_down(KEY_A)) {
            m_editor.camera.position = v3f_add(m_editor.camera.position, v3f_scale(v3f_cross(m_editor.camera.front, m_editor.camera.up), 0.5 * dt));
        }
        if (is_key_down(KEY_D)) {
            m_editor.camera.position = v3f_sub(m_editor.camera.position, v3f_scale(v3f_cross(m_editor.camera.front, m_editor.camera.up), 0.5 * dt));
        }
        f32 x_offset = mouse_delta.x;
        f32 y_offset = -mouse_delta.y;

        f32 sensitivity = 0.05f;
        x_offset *= sensitivity;
        y_offset *= sensitivity;

        m_editor.camera.yaw   += x_offset;
        m_editor.camera.pitch += y_offset;

        if (m_editor.camera.pitch > 89.0f)
            m_editor.camera.pitch = 89.0f;
        if (m_editor.camera.pitch < -89.0f)
            m_editor.camera.pitch = -89.0f;

        V3f direction;
        direction.x = cos(deg_to_rad(m_editor.camera.yaw)) * cos(deg_to_rad(m_editor.camera.pitch));
        direction.y = sin(deg_to_rad(m_editor.camera.pitch));
        direction.z = -sin(deg_to_rad(m_editor.camera.yaw)) * cos(deg_to_rad(m_editor.camera.pitch));
        m_editor.camera.front = v3f_normalize(direction);

    }
}
