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

// ---- MicroUI panel ---------------------------------------------------------

static void draw_skin_panel(void)
{
    mu_Context *ui = platform_ctx.ui;

    if (mu_begin_window(ui, "Skin Editor",
                        mu_rect(GAME_WIDTH - 220, 0, 220, GAME_HEIGHT / 2)))
    {
        // Mode toggle
        mu_layout_row(ui, 2, (int[]){100, -1}, 24);
        if (mu_button(ui, "Select"))
            m_editor.mode = MODEL_EDITOR_MODE_SELECT;
        if (mu_button(ui, "Paint"))
            m_editor.mode = MODEL_EDITOR_MODE_PAINT;

        // Current mode label
        mu_layout_row(ui, 1, (int[]){-1}, 18);
        mu_label(ui, m_editor.mode == MODEL_EDITOR_MODE_PAINT
                         ? "Mode: PAINT  [G to toggle]"
                         : "Mode: SELECT [G to toggle]");

        // Group selector — 4 per row, colour-coded via button labels
        mu_layout_row(ui, 1, (int[]){-1}, 18);
        mu_label(ui, "Active group:");

        char label[8];
        mu_layout_row(ui, 4, (int[]){44, 44, 44, -1}, 28);
        for (u16 g = 0; g < 16; g++) {
            snprintf(label, sizeof(label), "%s%d", (g == m_editor.active_group) ? ">" : " ", g);
            if (mu_button(ui, label))
                m_editor.active_group = g;
        }

        // Stats
        if (m_editor.selected) {
            mu_layout_row(ui, 1, (int[]){-1}, 18);
            char info[64];
            snprintf(info, sizeof(info), "Base verts: %u", m_editor.selected->base_count);
            mu_label(ui, info);

            // Count how many base positions are in the active group
            u32 painted = 0;
            for (u32 i = 0; i < m_editor.selected->base_count; i++) {
                if (m_editor.selected->skin_groups[i] == m_editor.active_group)
                    painted++;
            }
            snprintf(info, sizeof(info), "In group %d: %u", m_editor.active_group, painted);
            mu_label(ui, info);
        }

        // Save current skin groups to data/<model>.skin
        mu_layout_row(ui, 1, (int[]){-1}, 24);
        if (mu_button(ui, "Save Skin")) {
            char path[300];
            snprintf(path, sizeof(path), "data/%s.skin", m_editor.model_name);
            skin_save(path, m_editor.selected);
        }

        mu_end_window(ui);
    }
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

// MicroUI panel for the anim editor.
// AI-GENERATED
static void draw_anim_panel(void)
{
    mu_Context *ui = platform_ctx.ui;
    if (!mu_begin_window(ui, "Anim Editor",
                         mu_rect(0, 0, 220, GAME_HEIGHT * 3 / 4)))
        return;

    mu_layout_row(ui, 1, (int[]){-1}, 18);
    mu_label(ui, "Mode: ANIM  [G to change]");
    mu_label(ui, "R = toggle rotate/translate");

    // Build skel from currently-painted groups
    mu_layout_row(ui, 1, (int[]){-1}, 24);
    if (mu_button(ui, "Build Skel from Groups")) {
        anim_editor_build_skel();
        compute_joint_centers();
    }

    if (!m_editor.edit_base) { mu_end_window(ui); return; }

    // Joint selector — only show groups that have verts
    mu_layout_row(ui, 1, (int[]){-1}, 18);
    char info[64];
    snprintf(info, sizeof(info), "Joints: %u  [R=rot/trans]", m_editor.edit_base->count);
    mu_label(ui, info);

    char label[8];
    mu_layout_row(ui, 4, (int[]){44, 44, 44, -1}, 28);
    for (u16 g = 0; g < 16; g++) {
        if (!m_editor.joint_has_verts[g]) continue;
        snprintf(label, sizeof(label), "%s%d",
                 (g == (u16)m_editor.selected_joint) ? ">" : " ", g);
        if (mu_button(ui, label)) {
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

    // Record / clear
    mu_layout_row(ui, 2, (int[]){100, -1}, 24);
    if (mu_button(ui, "Add Frame"))
        anim_editor_add_frame();
    if (mu_button(ui, "Clear Pose")) {
        for (s32 j = 0; j < 16; j++) {
            m_editor.edit_pose[j] = (Transform){
                .r = quat_identity(), .t = v3f(0,0,0), .s = v3f(1,1,1)
            };
        }
        if (m_editor.edit_base)
            anim_apply_pose(m_editor.edit_base, m_editor.edit_pose, m_editor.selected);
    }

    // Frame count
    mu_layout_row(ui, 1, (int[]){-1}, 18);
    snprintf(info, sizeof(info), "Recorded frames: %d",
             (s32)arrlen(m_editor.edit_anim->frames));
    mu_label(ui, info);

    // Sequence name + duration
    mu_label(ui, "Sequence name:");
    mu_layout_row(ui, 1, (int[]){-1}, 24);
    mu_textbox(ui, m_editor.seq_name, sizeof(m_editor.seq_name));

    mu_layout_row(ui, 1, (int[]){-1}, 18);
    mu_label(ui, "ms per frame:");
    mu_layout_row(ui, 1, (int[]){-1}, 24);
    mu_slider(ui, &m_editor.frame_dur_ms, 50.0f, 1000.0f);

    // Save both .skel and .anim
    mu_layout_row(ui, 1, (int[]){-1}, 24);
    if (mu_button(ui, "Save Skel + Anim")) {
        char skel_path[300], anim_path[300];
        snprintf(skel_path, sizeof(skel_path), "data/%s.skel", m_editor.model_name);
        snprintf(anim_path, sizeof(anim_path), "data/%s.anim", m_editor.model_name);
        write_skel_file(skel_path);
        write_anim_file(anim_path);
    }

    mu_end_window(ui);
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

// Shared MicroUI command dispatcher — same logic in both model_editor_frame and
// model_editor_anim_frame.  Call after mu_end().
// AI-GENERATED
static void dispatch_ui_commands(void)
{
    mu_Command *cmd = NULL;
    f32 ui_z = 0.4f;
    while (mu_next_command(platform_ctx.ui, &cmd)) {
        switch (cmd->type) {
            case MU_COMMAND_ICON: {
                char icon_char;
                switch (cmd->icon.id) {
                    case 1: icon_char = 'X'; break;
                    case 2: icon_char = 'V'; break;
                    case 3: icon_char = '>'; break;
                    case 4: icon_char = 'v'; break;
                    default: icon_char = '?'; break;
                }
                char icon_str[2] = {icon_char, '\0'};
                draw_text(m_editor.hud_font, icon_str,
                          v2i(cmd->icon.rect.x, cmd->icon.rect.y),
                          16, mu_to_color(cmd->icon.color));
            } break;
            case MU_COMMAND_TEXT:
                if ((unsigned char)cmd->text.str[0] >= 32)
                    draw_text(m_editor.hud_font, cmd->text.str,
                              v2i(cmd->text.pos.x, cmd->text.pos.y),
                              12, mu_to_color(cmd->text.color));
                break;
            case MU_COMMAND_RECT:
                draw_recs32(mu_to_rec(cmd->rect.rect), ui_z,
                            mu_to_color(cmd->rect.color));
                ui_z -= 0.001f;
                break;
        }
    }
}

void model_editor_frame(void)
{
    if (!m_editor.selected) return;

    // AI-GENERATED: sync renderer.camera ← m_editor.camera so that
    // draw_rectangle3d (used by the gizmo) uses the same view as the model mesh.
    renderer.camera = m_editor.camera;

    if (is_mouse_button_down(MOUSEBUTTON_RIGHT) || is_key_down(KEY_V)) 
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
            case MODEL_EDITOR_MODE_SELECT: m_editor.mode = MODEL_EDITOR_MODE_PAINT; break;
            case MODEL_EDITOR_MODE_PAINT:  m_editor.mode = MODEL_EDITOR_MODE_ANIM;  break;
            case MODEL_EDITOR_MODE_ANIM:   m_editor.mode = MODEL_EDITOR_MODE_SELECT; break;
        }
        m_editor.hovered_tri = -1;
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

        if (is_mouse_button_down(MOUSEBUTTON_LEFT) && m_editor.hovered_tri >= 0)
            paint_triangle(m_editor.selected, m_editor.hovered_tri, m_editor.active_group);
    }

    mu_begin(platform_ctx.ui);
    draw_skin_panel();
    mu_end(platform_ctx.ui);
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

    // R toggles rotate ↔ translate on the active joint's gizmo
    if (is_key_pressed(KEY_R) && m_editor.selected_joint >= 0) {
        m_editor.joint_gizmo.mode =
            (m_editor.joint_gizmo.mode == GIZMO_MODE_TRANSLATE)
            ? GIZMO_MODE_ROTATE
            : GIZMO_MODE_TRANSLATE;
        m_editor.gizmo_axis = -1;
    }

    // Rebuild AABBs every frame so they track the gizmo position
    if (m_editor.selected_joint >= 0)
        gizmo_update(&m_editor.joint_gizmo);

    // ---- mouse press: pick a gizmo axis ----
    if (mouse_pressed && m_editor.selected_joint >= 0) {
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

    // ---- drag: update pose and apply live preview ----
    if (m_editor.gizmo_axis >= 0 && mouse_down && m_editor.selected_joint >= 0) {
        s32 j = m_editor.selected_joint;

        if (m_editor.joint_gizmo.mode == GIZMO_MODE_ROTATE) {
            f32 angle = 0.0f;
            switch (m_editor.gizmo_axis) {
                case GIZMO_AXIS_X: angle = -mouse_delta.y * 0.01f; break;
                case GIZMO_AXIS_Y: angle =  mouse_delta.x * 0.01f; break;
                case GIZMO_AXIS_Z: angle =  mouse_delta.x * 0.01f; break;
                default: break;
            }
            gizmo_rotation_modify(&m_editor.joint_gizmo, (Gizmo_Axis)m_editor.gizmo_axis, angle);
            m_editor.edit_pose[j].r = mat3_to_quat(gizmo_get_rotation(&m_editor.joint_gizmo));
        } else {
            // World-space delta from gizmo; negate z to convert to model-space
            V3f world_delta = gizmo_translation_modify(
                &m_editor.joint_gizmo, (Gizmo_Axis)m_editor.gizmo_axis,
                v2f_scale(mouse_delta, 0.01f));
            V3f model_delta = v3f(world_delta.x, world_delta.y, -world_delta.z);
            m_editor.edit_pose[j].t = v3f_add(m_editor.edit_pose[j].t, model_delta);
            // Move gizmo visually so it follows the joint
            m_editor.joint_gizmo.position = v3f_add(m_editor.joint_gizmo.position, world_delta);
        }

        if (m_editor.edit_base)
            anim_apply_pose(m_editor.edit_base, m_editor.edit_pose, m_editor.selected);
    }

    if (mouse_released)
        m_editor.gizmo_axis = -1;

    // ---- draw model with group colours using live vertex positions ----
    draw_anim_groups_live(m_editor.selected);

    // ---- draw small triangle markers at joint centres ----
    // Markers are pushed as view-space triangles (renderer.camera already applied by
    // draw_anim_groups_live logic above, but markers go through renderer_push_triangle
    // which expects view-space).
    Mat4 view = camera_matrix(renderer.camera);
    for (u16 g = 0; g < 16; g++) {
        if (!m_editor.joint_has_verts[g]) continue;
        V3f p = v3f_translate_by_mat4(m_editor.joint_centers[g], view);
        if (p.z < NEAR) continue;
        Color c   = ((s32)g == m_editor.selected_joint)
                  ? (Color){255, 255, 255, 255}
                  : GROUP_PALETTE[g % 16];
        Color col3[3] = {c, c, c};
        V3f   uvs3[3] = {0};
        f32 s = 0.025f;
        renderer_push_triangle(
            v3f(p.x,     p.y + s, p.z),
            v3f(p.x - s, p.y - s, p.z),
            v3f(p.x + s, p.y - s, p.z),
            col3, uvs3, NULL, TRIANGLE_WRITE_OVER_Z | TRIANGLE_NO_CULLING);
    }

    // ---- draw gizmo if a joint is selected ----
    if (m_editor.selected_joint >= 0) {
        m_editor.joint_gizmo.active_axis = m_editor.gizmo_axis;
        gizmo_draw(&m_editor.joint_gizmo);
    }

    // ---- MicroUI ----
    mu_begin(platform_ctx.ui);
    draw_anim_panel();
    mu_end(platform_ctx.ui);
    dispatch_ui_commands();
}

void model_editor_camera_update(void)
{
	V2f mouse_delta = get_mouse_delta();

    if (is_mouse_button_down(MOUSEBUTTON_MIDDLE)) {
        if (is_key_down(KEY_W)) {
            m_editor.camera.position = v3f_add(m_editor.camera.position, v3f_scale(m_editor.camera.front, 0.005));
        }
        if (is_key_down(KEY_S)) {
            m_editor.camera.position = v3f_add(m_editor.camera.position, v3f_scale(m_editor.camera.front, -0.005));
        }
        if (is_key_down(KEY_A)) {
            m_editor.camera.position = v3f_add(m_editor.camera.position, v3f_scale(v3f_cross(m_editor.camera.front, m_editor.camera.up), 0.005));
        }
        if (is_key_down(KEY_D)) {
            m_editor.camera.position = v3f_sub(m_editor.camera.position, v3f_scale(v3f_cross(m_editor.camera.front, m_editor.camera.up), 0.005));
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
