void editor_master_panel(void);


void editor_init(void)
{
    editor.arena = arena_alloc();
    editor.camera.front = v3f_normalize(v3f_sub(editor.camera.target, editor.camera.position));

    V3f pos = editor.camera.position;
    V3f target = editor.camera.target;
    editor.camera.pitch = tanf((pos.y - target.y) / (pos.z - target.z));
    editor.camera.yaw = -90;


    V3f direction;
    direction.x = cos(deg_to_rad(editor.camera.yaw)) * cos(deg_to_rad(editor.camera.pitch));
    direction.y = sin(deg_to_rad(editor.camera.pitch));
    direction.z = -sin(deg_to_rad(editor.camera.yaw)) * cos(deg_to_rad(editor.camera.pitch));
    editor.camera.front = v3f_normalize(direction);
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
        e->hit = (i == editor.selected_entity);
    }

    if (editor.gizmo.attached) {
        gizmo_update(&editor.gizmo);
    }


    //
    // 1) Handle click selection
    //
    if (editor.mouse_pressed) {
        editor.selected_axis = -1;

        // Gizmo gets priority over entity selection.
        if (editor.gizmo.attached) {
            f32 closest = FLT_MAX;

            for (s32 i = 0; i < GIZMO_AXIS_COUNT; i++) {
                RayCollision collision = get_raycollision_box(editor.mouse_ray, editor.gizmo.aabbs[i]);
                if (collision.hit && collision.distance < closest) {
                    closest = collision.distance;
                    editor.selected_axis = i;
                    editor.clicked_gizmo = true;
                }
            }

            if (editor.clicked_gizmo) {
                log_info("Gizmo axis %d hit", editor.selected_axis);
            }
        }

        // Only try entity picking if we did not click the gizmo.
        if (!editor.clicked_gizmo) {
            s32 hit_tile = -1;
            s32 hit_entity = -1;
            float closest = FLT_MAX;

            for (s32 i = 0; i < arrlen(editor.scene->dynamic_entities); i++) {
                Entity *e = &editor.scene->dynamic_entities[i];
                e->update_disabled = true;
                RayCollision collision = entity_mouse_ray_collision(e, editor.mouse_ray);
                if (collision.hit && collision.distance < closest) {
                    closest = collision.distance;
                    hit_entity = i;
                }
            }

            if (hit_entity >= 0) {
                editor.clicked_entity = true;
                editor.selected_entity = hit_entity;
                editor.gizmo.attached = true;
                editor.gizmo.position = editor.scene->dynamic_entities[hit_entity].position;
            } else {
                for (s32 i = 0; i < arrlen(editor.scene->tiles); i++) {
                    Tile *e = &editor.scene->tiles[i];
                    RayCollision collision = tile_mouse_ray_collision(e, editor.mouse_ray);
                    if (collision.hit && collision.distance < closest) {
                        closest = collision.distance;
                        hit_tile = i;
                    }
                }

                if (hit_tile >= 0) {
                    editor.clicked_tile = true;
                    editor.selected_tile = hit_tile;
                    show_demo = true;
                }
            }
        }
    }

    if (is_key_pressed(KEY_Q)) {
        editor.selected_entity = -1;
        editor.selected_tile = -1;
        editor.selected_axis = -1;
        editor.gizmo.attached = false;
        show_demo = false;
    }

    if (is_key_pressed(KEY_R) && editor.gizmo.attached) {
        editor.gizmo.mode = (editor.gizmo.mode == GIZMO_MODE_TRANSLATE)
                      ? GIZMO_MODE_ROTATE
                      : GIZMO_MODE_TRANSLATE;
        editor.selected_axis = -1;
    }

    //
    // 2) Drag currently selected gizmo axis
    //
    if (editor.selected_axis >= 0 && editor.mouse_down && editor.selected_entity >= 0) {
        Entity *e = &editor.scene->dynamic_entities[editor.selected_entity];

        if (editor.gizmo.mode == GIZMO_MODE_ROTATE) {
            f32 angle = 0.0f;
            switch (editor.selected_axis) {
                case GIZMO_AXIS_X: angle = -editor.mouse_delta.y * 0.01f; break;
                case GIZMO_AXIS_Y: angle = editor.mouse_delta.x * 0.01f; break;
                case GIZMO_AXIS_Z: angle = editor.mouse_delta.x * 0.01f; break;
                default: break;
            }
            gizmo_rotation_modify(&editor.gizmo, (Gizmo_Axis)editor.selected_axis, angle);
            e->rotation = gizmo_get_rotation(&editor.gizmo);
        } else {
            V3f delta = gizmo_translation_modify(
                &editor.gizmo,
                editor.selected_axis,
                v2f_scale(editor.mouse_delta, 0.01f)
            );
            e->position =  v3f_add(e->position, delta);
            e->target = e->position;
            editor.gizmo.position = e->position;
        }
    }

    //
    // 3) Releasing mouse ends gizmo drag but keeps entity selected
    //
    if (editor.mouse_released) {
        if (editor.gizmo.mode == GIZMO_MODE_TRANSLATE && editor.selected_entity >= 0) {
            Entity *e = &editor.scene->dynamic_entities[editor.selected_entity];
            e->position = v3f(roundf(e->position.x), roundf(e->position.y), roundf(e->position.z));
            editor.gizmo.position = e->position;
        }
        editor.selected_axis = -1;
    }
    for (s32 i = 0; i < arrlen(editor.scene->tiles); i++) {
        tile_draw(&editor.scene->tiles[i]);
    }

    for (int i = 0; i < arrlen(editor.scene->dynamic_entities); i++) {
        Entity e = editor.scene->dynamic_entities[i];
        entity_draw(&e);
        if (e.hit) {
                editor.gizmo.active_axis = editor.selected_axis;
                gizmo_draw(&editor.gizmo);
                show_demo = true;
        }
    }

    static bool entity_panel_open = true;
    if (is_key_pressed(KEY_F1))
        entity_panel_open = !entity_panel_open;

    struct nk_context *ctx = platform_ctx.ui;
    static const nk_flags panel_flags =
        NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_TITLE;

    if (show_demo && entity_panel_open && !console.open) {
        if (nk_begin(ctx, "Entity",
                     nk_rect(0, 40, platform_ctx.width / 4, platform_ctx.height / 2), panel_flags))
        {
            if (editor.selected_entity >= 0) {
                Entity *e = &editor.scene->dynamic_entities[editor.selected_entity];
                if (nk_tree_push(ctx, NK_TREE_TAB, "Mesh", NK_MAXIMIZED)) {
                    nk_layout_row_dynamic(ctx, 20, 1);
                    nk_label_wrap(ctx, "Change mesh");

                    static int selected = 0;


                    int count = shlen(assets);
                    const char **keys = (const char **)alloca(count * sizeof(char *));
                    for (int i = 0; i < count; i++) {
                        keys[i] = assets[i].key;
                    }
                    nk_layout_row_dynamic(ctx, 25, 1);
                    selected = nk_combo(ctx, keys, count, selected, 25, nk_vec2(200, 150));

                    if (selected != 0) {
                        e->model = shget(assets, keys[selected]);
                        selected = 0;
                    }

                    nk_label_wrap(ctx, e->model_tag);
                    snprintf(buf, 256, "Entity ID: %d", editor.selected_entity);

                    nk_label_wrap(ctx, "scale");
                    static f32 scale = 1.0;
                    nk_slider_float(ctx, 0, &scale, 1, 0.01f);
                    e->scale = scale;

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

            } else if (editor.selected_tile >= 0) {
                Tile *t = &editor.scene->tiles[editor.selected_tile];
                if (nk_tree_push(ctx, NK_TREE_TAB, "Tile", NK_MAXIMIZED)) {
                    nk_label_wrap(ctx, "Tile Color Picker");
                    struct nk_color tcolor = {t->color.r, t->color.g, t->color.b, t->color.a};
                    if (nk_combo_begin_color(ctx, tcolor, nk_vec2(nk_widget_width(ctx), 200))) {
                        nk_layout_row_dynamic(ctx, 120, 1);
                        struct nk_colorf color = nk_color_picker(ctx,
                                                                 (struct nk_colorf){
                                                                     .r = tcolor.r / 255.0,  
                                                                     .g = tcolor.g / 255.0,  
                                                                     .b = tcolor.b / 255.0,  
                                                                     .a = tcolor.a / 255.0,  
                                                                 },
                                                                 NK_RGBA);
                        nk_layout_row_dynamic(ctx, 25, 1);
                        color.r = nk_propertyf(ctx, "#R:", 0, color.r, 1.0f, 0.01f,0.005f);
                        color.g = nk_propertyf(ctx, "#G:", 0, color.g, 1.0f, 0.01f,0.005f);
                        color.b = nk_propertyf(ctx, "#B:", 0, color.b, 1.0f, 0.01f,0.005f);
                        color.a = nk_propertyf(ctx, "#A:", 0, color.a, 1.0f, 0.01f,0.005f);

                        t->color.r = color.r*255;
                        t->color.g = color.g*255;
                        t->color.b = color.b*255;
                        t->color.a = color.a*255;
                        nk_combo_end(ctx);
                    }
                    nk_tree_pop(ctx);
                }

            } else {

                snprintf(buf, 256, "No selected Entity");
                nk_layout_row_dynamic(ctx, 20, 1);
                nk_label_wrap(ctx, buf);
                if (nk_tree_push(ctx, NK_TREE_TAB, "Spawners", NK_MAXIMIZED)) {
                    nk_layout_row_dynamic(ctx, 20, 1);

                    nk_tree_pop(ctx);
                }
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
                draw_text(gs.font, t->string, v2i(t->x, t->y), 20, c);
            } break;
            case NK_COMMAND_IMAGE: {
                const struct nk_command_image *t = (const struct nk_command_image *)cmd;
                Texture tex = {
                    .id = t->img.handle.id,  
                    .data = t->img.handle.ptr,  
                    .width = t->img.w,
                    .height = t->img.h,
                };
                draw_texture(&tex, (Recs32){.x = t->x, .y = t->y, .w = t->w, .h = t->h});
            } break;
            case NK_COMMAND_RECT_MULTI_COLOR: {
                const struct nk_command_rect_multi_color *t = (const struct nk_command_rect_multi_color *)cmd;
                Color left = {
                    t->left.r,
                    t->left.g,
                    t->left.b,
                    t->left.a,
                };
                Color top = {
                    t->top.r,
                    t->top.g,
                    t->top.b,
                    t->top.a,
                };
                Color bottom = {
                    t->bottom.r,
                    t->bottom.g,
                    t->bottom.b,
                    t->bottom.a,
                };
                Color right = {
                    t->right.r,
                    t->right.g,
                    t->right.b,
                    t->right.a,
                };
                draw_multitriangle(
                                   v3f(t->x, t->y, 0),
                                   v3f(t->x, t->y + t->h, 0),
                                   v3f(t->x + t->w, t->y + t->h, 0),
                                   left,
                                   top,
                                   right,
                                   0);
                draw_multitriangle(
                                   v3f(t->x, t->y, 0),
                                   v3f(t->x + t->w, t->y + t->h, 0),
                                   v3f(t->x + t->w, t->y, 0),
                                   left,
                                   right,
                                   bottom,
                                   0);
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
    s->window.background        = nk_rgb(37, 35, 35);
    s->window.fixed_background  = nk_style_item_color(nk_rgb(37, 35, 35));
    s->window.border_color      = nk_rgb(112, 121, 140);

    /* Button */
    s->button.normal     = nk_style_item_color(nk_rgb(50, 50, 50));
    s->button.hover      = nk_style_item_color(nk_rgb(70, 70, 70));
    s->button.active     = nk_style_item_color(nk_rgb( 0,160,255));
    s->button.text_normal = nk_rgb(218, 210, 188);
    s->button.text_hover  = nk_rgb(245, 241, 237);
    s->button.text_active = nk_rgb(255, 255, 255);
    s->button.border_color = nk_rgb(169, 153, 133);
    s->button.border       = 1.0f;
    s->button.rounding     = 8.0f;

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
                    if (nk_menu_item_label(ctx, "Open", NK_TEXT_LEFT)) {
                    }
                    if (nk_menu_item_label(ctx, "Save", NK_TEXT_LEFT)) {
                        scene_save(editor.scene, str8_to_cstring(editor.arena, editor.scene->name));
                    }
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
