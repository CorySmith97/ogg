void editor_master_panel(void);


void editor_init(void)
{
    editor.camera.front = v3f_normalize(v3f_sub(editor.camera.target, editor.camera.position));
}

void editor_set_scene(Scene *scene)
{
    editor.scene = scene;
}

void editor_update(void)
{
  
}

void editor_draw(void)
{
    char buf[256];

    editor.mouse_scroll = get_mouse_scroll();
    editor.mouse_pos    = get_mouse_pos();
    editor.mouse_delta  = get_mouse_delta();
    editor.mouse_ray    = get_mouse_ray(&editor.camera, editor.mouse_pos);

    // Suggested defaults somewhere during init/reset:

    editor.mouse_pressed  = is_mouse_button_pressed(MOUSEBUTTON_LEFT);
    editor.mouse_down     = is_mouse_button_down(MOUSEBUTTON_LEFT);
    editor.mouse_released = is_mouse_button_released(MOUSEBUTTON_LEFT);
    editor.clicked_gizmo  = false;
    editor.clicked_entity = false;


    // TODO set pose and dont update
    for (s32 i = 0; i < arrlen(editor.scene->dynamic_entities); i++) {
        Entity *e = &editor.scene->dynamic_entities[i];
        e->hit = (i == gs.selected_entity);
    }

    if (editor.gizmo.attached) {
        gizmo_update(&editor.gizmo);
    }

    //
    // 1) Handle click selection
    //
    if (editor.mouse_pressed) {
        gs.selected_axis = -1;

        // Gizmo gets priority over entity selection.
        if (gs.gizmo.attached) {
            f32 closest = FLT_MAX;

            for (s32 i = 0; i < GIZMO_AXIS_COUNT; i++) {
                RayCollision collision = get_raycollision_box(editor.mouse_ray, gs.gizmo.aabbs[i]);
                if (collision.hit && collision.distance < closest) {
                    closest = collision.distance;
                    gs.selected_axis = i;
                    editor.clicked_gizmo = true;
                }
            }

            if (editor.clicked_gizmo) {
                log_info("Gizmo axis %d hit", gs.selected_axis);
            }
        }

        // Only try entity picking if we did not click the gizmo.
        if (!editor.clicked_gizmo) {
            s32 hit_entity = -1;
            float closest = FLT_MAX;

            for (s32 i = 0; i < arrlen(editor.scene->dynamic_entities); i++) {
                Entity *e = &editor.scene->dynamic_entities[i];
                e->update_disabled = true;
                RayCollision collision = entity_mouse_ray_collision(e, editor.mouse_ray);
                if (collision.hit && collision.distance < closest) {
                    closest = collision.distance;
                    hit_entity = i;
                    log_info("This his %d", i);
                }
            }

            if (hit_entity >= 0) {
                editor.clicked_entity = true;
                gs.selected_entity = hit_entity;
                gs.gizmo.attached = true;
                gs.gizmo.position = editor.scene->dynamic_entities[hit_entity].position;
            } 
        }
    }

    if (is_key_pressed(KEY_Q)) {
        gs.selected_entity = -1;
        gs.selected_axis = -1;
        gs.gizmo.attached = false;
    }

    if (is_key_pressed(KEY_R) && gs.gizmo.attached) {
        gs.gizmo.mode = (gs.gizmo.mode == GIZMO_MODE_TRANSLATE)
                      ? GIZMO_MODE_ROTATE
                      : GIZMO_MODE_TRANSLATE;
        gs.selected_axis = -1;
    }

    //
    // 2) Drag currently selected gizmo axis
    //
    if (gs.selected_axis >= 0 && editor.mouse_down && gs.selected_entity >= 0) {
        Entity *e = &editor.scene->dynamic_entities[gs.selected_entity];

        if (gs.gizmo.mode == GIZMO_MODE_ROTATE) {
            f32 angle = 0.0f;
            switch (gs.selected_axis) {
                case GIZMO_AXIS_X: angle = -editor.mouse_delta.y * 0.01f; break;
                case GIZMO_AXIS_Y: angle = editor.mouse_delta.x * 0.01f; break;
                case GIZMO_AXIS_Z: angle = editor.mouse_delta.x * 0.01f; break;
                default: break;
            }
            gizmo_rotation_modify(&gs.gizmo, (Gizmo_Axis)gs.selected_axis, angle);
            e->rotation = gizmo_get_rotation(&gs.gizmo);
        } else {
            V3f delta = gizmo_translation_modify(
                &gs.gizmo,
                gs.selected_axis,
                v2f_scale(editor.mouse_delta, 0.01f)
            );
            e->position =  v3f_add(e->position, delta);
            e->target = e->position;
            gs.gizmo.position = e->position;
        }
    }

    //
    // 3) Releasing mouse ends gizmo drag but keeps entity selected
    //
    if (editor.mouse_released) {
        if (gs.gizmo.mode == GIZMO_MODE_TRANSLATE && gs.selected_entity >= 0) {
            Entity *e = &editor.scene->dynamic_entities[gs.selected_entity];
            e->position = v3f(roundf(e->position.x), roundf(e->position.y), roundf(e->position.z));
            gs.gizmo.position = e->position;
        }
        gs.selected_axis = -1;
    }
    for (s32 i = 0; i < arrlen(editor.scene->tiles); i++) {
        tile_draw(&editor.scene->tiles[i]);
    }

    for (int i = 0; i < arrlen(editor.scene->dynamic_entities); i++) {
        Entity e = editor.scene->dynamic_entities[i];
        entity_draw(&e);
        if (e.hit) {
            if (!gs.camera_moving) {
                gs.gizmo.active_axis = gs.selected_axis;
                gizmo_draw(&gs.gizmo);
                show_demo = true;
            }
        }
    }

    static bool entity_panel_open = true;
    if (is_key_pressed(KEY_F1))
        entity_panel_open = !entity_panel_open;

    struct nk_context *ctx = platform_ctx.ui;
    static const nk_flags panel_flags =
        NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_TITLE;

    if (show_demo && entity_panel_open) {
        if (nk_begin(ctx, "Entity",
                     nk_rect(0, 40, SCREEN_WIDTH / 4, SCREEN_HEIGHT / 2), panel_flags))
        {
            if (gs.selected_entity >= 0) {
                Entity *e = &editor.scene->dynamic_entities[gs.selected_entity];
                if (nk_tree_push(ctx, NK_TREE_TAB, "Mesh", NK_MAXIMIZED)) {
                    nk_layout_row_dynamic(ctx, 20, 1);
                    nk_label_wrap(ctx, "Change mesh");
                    static int selected = 0;


                    int count = shlen(gltf_assets);
                    const char **keys = (const char **)alloca(count * sizeof(char *));
                    for (int i = 0; i < count; i++) {
                        keys[i] = gltf_assets[i].key;
                    }
                    nk_layout_row_dynamic(ctx, 25, 1);
                    selected = nk_combo(ctx, keys, count, selected, 25, nk_vec2(200, 150));
                    nk_label_wrap(ctx, e->model_tag);
                    snprintf(buf, 256, "Entity ID: %d", gs.selected_entity);
                    static f32 rotation = 0;
                    nk_layout_row_dynamic(ctx, 22, 1);
                    nk_slider_float(ctx, 0, &rotation, (float)M_TAU, 0.01f);
                    e->rotation = rotation_y(rotation);
                    nk_layout_row_dynamic(ctx, 20, 1);
                    nk_label_wrap(ctx, buf);
                    snprintf(buf, 256, "%.1f, %.1f, %.1f",
                             e->position.x, e->position.y, e->position.z);

                    nk_label_wrap(ctx, buf);
                    snprintf(buf, 256, "%.1f, %.1f, %.1f",
                             e->target.x, e->target.y, e->target.z);
                    nk_label_wrap(ctx, buf);

                    nk_tree_pop(ctx);
                }
                if (nk_tree_push(ctx, NK_TREE_TAB, "Game Info", NK_MAXIMIZED)) {
                    nk_layout_row_dynamic(ctx, 20, 1);
                    nk_label_wrap(ctx, "Race: ");
                    nk_label_wrap(ctx, "Base Class: ");
                    if (nk_tree_push(ctx, NK_TREE_TAB, "Attributes", NK_MAXIMIZED)) {
                        nk_layout_row_dynamic(ctx, 20, 1);
                        nk_label_wrap(ctx, "Strength: ");
                        nk_property_float(ctx, "#strength", 0,
                                          &e->attributes.strength, 40, 1, 1);
                        nk_label_wrap(ctx, "Base Class: ");
                        nk_tree_pop(ctx);
                    }
                    nk_tree_pop(ctx);

                }

            } else if (gs.selected_tile >= 0) {
                Tile *t = &editor.scene->tiles[gs.selected_tile];
                if (nk_tree_push(ctx, NK_TREE_TAB, "Mesh", NK_MAXIMIZED)) {
                    nk_tree_pop(ctx);
                }

            } else {

                snprintf(buf, 256, "No selected Entity");
                nk_layout_row_dynamic(ctx, 20, 1);
                nk_label_wrap(ctx, buf);
            }
        }
        nk_end(ctx);
    }

    editor_master_panel();

    const struct nk_command *cmd;
    float ui_z = 0.2f;
    nk_foreach(cmd, ctx) {
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
                draw_text(gs.font, t->string, v2i(t->x, t->y), 16, c);
            } break;
            default: break;
        }
    }
    nk_clear(ctx);
    SectionStart("Editor Flush");
    if (is_key_pressed(KEY_N)) {
        log_debug("Triangled rendered %zu", triangles->len);
    }
    renderer_flush();
    SectionEnd("Editor Flush");

}

void editor_master_panel(void)
{
    struct nk_context *ctx = platform_ctx.ui;
    static const nk_flags panel_flags = NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE;
    struct nk_style *s = &ctx->style;

    /* Window */
    s->window.background        = nk_rgb(30, 130, 120);
    s->window.fixed_background  = nk_style_item_color(nk_rgb(30, 130, 120));
    s->window.border_color      = nk_rgb(60, 60, 60);

    /* Button */
    s->button.normal     = nk_style_item_color(nk_rgb(50, 50, 50));
    s->button.hover      = nk_style_item_color(nk_rgb(70, 70, 70));
    s->button.active     = nk_style_item_color(nk_rgb( 0,160,255));
    s->button.text_normal = nk_rgb(210, 210, 210);
    s->button.text_hover  = nk_rgb(55, 255, 255);
    s->button.text_active = nk_rgb(255, 255, 255);
    s->button.border_color = nk_rgb(60, 60, 60);
    s->button.border       = 1.0f;
    s->button.rounding     = 3.0f;

    /* Slider */
    s->slider.bar_normal   = nk_rgb(40,  40,  40);
    s->slider.bar_hover    = nk_rgb(40,  40,  40);
    s->slider.bar_active   = nk_rgb(40,  40,  40);
    s->slider.bar_filled   = nk_rgb( 0, 160, 255);
    s->slider.cursor_normal = nk_style_item_color(nk_rgb( 0, 160, 255));
    s->slider.cursor_hover  = nk_style_item_color(nk_rgb(30, 180, 255));
    s->slider.cursor_active = nk_style_item_color(nk_rgb( 0, 140, 220));


    if (nk_begin(ctx, "Window", nk_rect(0, 0, SCREEN_WIDTH, 30), 0))
        {
            /* ── Menu Bar ── */
            nk_menubar_begin(ctx);

            nk_layout_row_begin(ctx, NK_STATIC, 25, 2); /* 2 menus, 25px tall */

            /* "File" menu */
            nk_layout_row_push(ctx, 70);
            if (nk_menu_begin_label(ctx, "File", NK_TEXT_LEFT, nk_vec2(120, 200)))
                {
                    nk_layout_row_dynamic(ctx, 25, 1);
                    if (nk_menu_item_label(ctx, "New",  NK_TEXT_LEFT)) { /* handle New  */ }
                    if (nk_menu_item_label(ctx, "Open", NK_TEXT_LEFT)) { /* handle Open */ }
                    if (nk_menu_item_label(ctx, "Save", NK_TEXT_LEFT)) { /* handle Save */ }
                    nk_menu_end(ctx);
                }

            /* "Edit" menu */
            nk_layout_row_push(ctx, 60);
            if (nk_menu_begin_label(ctx, "Edit", NK_TEXT_LEFT, nk_vec2(120, 200)))
                {
                    nk_layout_row_dynamic(ctx, 25, 1);
                    if (nk_menu_item_label(ctx, "Cut",   NK_TEXT_LEFT)) { /* handle Cut   */ }
                    if (nk_menu_item_label(ctx, "Copy",  NK_TEXT_LEFT)) { /* handle Copy  */ }
                    if (nk_menu_item_label(ctx, "Paste", NK_TEXT_LEFT)) { /* handle Paste */ }
                    nk_menu_end(ctx);
                }

            nk_layout_row_end(ctx);
            nk_menubar_end(ctx);

            /* ... rest of window content ... */
        }
    nk_end(ctx);
}

void editor_camera_update(void)
{
	V2f mouse_delta = get_mouse_delta();

    if (is_key_down(KEY_LEFT_CONTROL)) {
        if (is_key_down(KEY_W)) {
            editor.camera.position = v3f_add(editor.camera.position, v3f_scale(editor.camera.front, editor.camera_speed));
        }
        if (is_key_down(KEY_S)) {
            editor.camera.position = v3f_add(editor.camera.position, v3f_scale(editor.camera.front, -editor.camera_speed));
        }
        if (is_key_down(KEY_A)) {
            editor.camera.position = v3f_add(editor.camera.position, v3f_scale(v3f_cross(editor.camera.front, editor.camera.up), editor.camera_speed));
        }
        if (is_key_down(KEY_D)) {
            editor.camera.position = v3f_sub(editor.camera.position, v3f_scale(v3f_cross(editor.camera.front, editor.camera.up), editor.camera_speed));
        }
        f32 x_offset =  editor.mouse_delta.x;
        f32 y_offset = -editor.mouse_delta.y;

        f32 sensitivity = 0.3f;
        x_offset *= sensitivity;
        y_offset *= sensitivity;

        editor.camera.yaw   += x_offset;
        editor.camera.pitch += y_offset;

        if (editor.camera.pitch > 89.0f)
            editor.camera.pitch = 89.0f;
        if (editor.camera.pitch < -89.0f)
            editor.camera.pitch = -89.0f;

        V3f direction;
        direction.x = cos(deg_to_rad(editor.camera.yaw)) * cos(deg_to_rad(editor.camera.pitch));
        direction.y = sin(deg_to_rad(editor.camera.pitch));
        direction.z = -sin(deg_to_rad(editor.camera.yaw)) * cos(deg_to_rad(editor.camera.pitch));
        editor.camera.front = v3f_normalize(direction);

    }
}
